// Example Cuckoo Backplate Communication Program
// Created by z3ugma
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <time.h>
#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include "linux/input.h"
#include <signal.h> // For Ctrl+C handling


const char *DEVICE_PATH = "/dev/ttyO2";
const char *FB_DEVICE_PATH = "/dev/fb0";
#define MAX_DATA_LEN 1024
#define MAX_RAW_MSG_LEN (MAX_DATA_LEN + 10) // Header(4) + Cmd(2) + Len(2) + CRC(2)

// --- Global Flags & Handles ---
volatile bool g_running = true;
bool g_print_raw = false;
int g_screen_fd = -1;

// --- Signal Handler for Ctrl+C ---
void signal_handler(int signum) {
    if (signum == SIGINT) {
        printf("\nCtrl+C detected. Shutting down gracefully...\n");
        g_running = false;
    }
}

// --- Structs and Enums ---
typedef enum {
    PARSER_STATE_WAIT_D5_1, PARSER_STATE_WAIT_D5_2, PARSER_STATE_WAIT_AA,
    PARSER_STATE_WAIT_96, PARSER_STATE_READ_CMD_L, PARSER_STATE_READ_CMD_H,
    PARSER_STATE_READ_LEN_L, PARSER_STATE_READ_LEN_H, PARSER_STATE_READ_DATA,
    PARSER_STATE_READ_CRC_L, PARSER_STATE_READ_CRC_H,
} ParserState;

typedef struct {
    uint16_t command;
    uint16_t data_len;
    uint8_t data[MAX_DATA_LEN];
    uint16_t crc;
    bool crc_ok;
} BackplateMessage;

// --- Screen Color Control ---
void set_screen_color(int screen_buffer, uint32_t color);
void update_screen_from_als(uint16_t lux_value);


// --- CRC Calculation ---
uint16_t crctab[0x100];
void init_crctab() {
    const uint16_t poly = 0x1021;
    for (int i = 0; i < 256; ++i) {
        uint16_t r = (uint16_t)i << 8;
        for (int j = 0; j < 8; ++j) {
            if (r & 0x8000) r = (r << 1) ^ poly; else r <<= 1;
        }
        crctab[i] = r;
    }
}
uint16_t crc16ccitt(const unsigned char *buf, size_t sz) {
    uint16_t crc = 0;
    for (size_t i = 0; i < sz; ++i) {
        crc = (crc << 8) ^ crctab[((crc >> 8) ^ buf[i]) & 0xff];
    }
    return crc;
}

// --- Helper Functions ---
char* get_timestamp() {
    static char buffer[30];
    struct timeval tv;
    gettimeofday(&tv, NULL);
    struct tm *tm_info = localtime(&tv.tv_sec);
    strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", tm_info);
    sprintf(buffer + 19, ".%03ld", tv.tv_usec / 1000);
    return buffer;
}

void print_hex(const unsigned char* buffer, size_t length) {
    for (size_t i = 0; i < length; ++i) printf("%02x ", buffer[i]);
}

void process_message(const BackplateMessage* msg); // Forward declaration
BackplateMessage* parse_byte(uint8_t byte); // Forward declaration

void process_message(const BackplateMessage* msg) {
    printf("%s RECV (CMD: 0x%04x, LEN: %u, CRC: %s) ", get_timestamp(), msg->command, msg->data_len, msg->crc_ok ? "OK" : "FAIL");
    
    if (msg->command == 0x0001) { // RESP_ASCII
        printf("ASCII: \"");
        for(int i=0; i < msg->data_len; ++i) {
            putchar(isprint(msg->data[i]) ? msg->data[i] : '.');
        }
        printf("\"\n");
        return;
    }

    switch(msg->command) {
        // Responses to Get Commands
        case 0x0018: case 0x0019: case 0x001b: case 0x001c: case 0x001d: case 0x001e: case 0x001f:
             printf("INFO: \"");
             for(int i=0; i < msg->data_len; ++i) putchar(isprint(msg->data[i]) ? msg->data[i] : '.');
             printf("\"");
             break;
        
        // Sensor Data
        case 0x0002: case 0x0022: // Temp/Humidity
            if (msg->data_len >= 4) {
                int16_t temp_cc = (msg->data[1] << 8) | msg->data[0];
                uint16_t hum_pm = (msg->data[3] << 8) | msg->data[2];
                printf("Temp/Hum: %.2f C, %.1f %%", (double)temp_cc / 100.0, (double)hum_pm / 10.0);
            }
            break;
        case 0x0005: // PIR Raw Data
            if (msg->data_len >= 4) {
                int16_t val1 = (msg->data[1] << 8) | msg->data[0];
                int16_t val2 = (msg->data[3] << 8) | msg->data[2];
                printf("PIR Raw Data -> val1: %d, val2: %d", val1, val2);
            }
            break;
        case 0x0007: // Proximity Sensor
            if (msg->data_len >= 4) {
                int16_t val1 = (msg->data[1] << 8) | msg->data[0];
                int16_t val2 = (msg->data[3] << 8) | msg->data[2];
                printf("Proximity Sensor -> Val1: %d, Val2: %d", val1, val2);
            } else if (msg->data_len >= 2) { // Handle the 2-byte case we are seeing
                int16_t val1 = (msg->data[1] << 8) | msg->data[0];
                printf("Proximity Sensor -> Value: %d", val1);
            }
            break;
        case 0x000a: // Ambient Light Sensor
             if (msg->data_len >= 2) {
                uint16_t lux = (msg->data[1] << 8) | msg->data[0];
                printf("Ambient Light -> %u lux", lux);
                update_screen_from_als(lux);
            }
            break;
        case 0x000b: // Backplate State Packet
            if (msg->data_len >= 16) {
                uint16_t vi = (msg->data[9] << 8) | msg->data[8];
                uint16_t vo = (msg->data[11] << 8) | msg->data[10];
                uint16_t vb = (msg->data[13] << 8) | msg->data[12];
                printf("Backplate State -> Vin: %.2fV, Vop: %.3fV, Vbat: %.3fV", (double)vi / 100.0, (double)vo / 1000.0, (double)vb / 1000.0);
            }
            break;
        case 0x000c: // Sensor Raw ADC Values
             if (msg->data_len >= 14) {
                uint16_t pir_raw = (msg->data[1] << 8) | msg->data[0];
                uint16_t alir_raw = (msg->data[11] << 8) | msg->data[10];
                uint16_t alvis_raw = (msg->data[13] << 8) | msg->data[12];
                printf("Sensor ADC -> PIR: %u, AL_IR: %u, AL_VIS: %u", pir_raw, alir_raw, alvis_raw);
            }
            break;
        case 0x0023: // PIR Motion Event
            if (msg->data_len >= 4) {
                int16_t val1 = (msg->data[1] << 8) | msg->data[0];
                int16_t val2 = (msg->data[3] << 8) | msg->data[2];
                if (val1 == 0 && val2 == 0) printf("PIR Event: Cleared");
                else printf("PIR Event: Motion Detected (vals: %d, %d)", val1, val2);
            }
            break;
        case 0x0025: // Proximity Event Data
            if (msg->data_len >= 4) {
                int16_t val1 = (msg->data[1] << 8) | msg->data[0];
                int16_t val2 = (msg->data[3] << 8) | msg->data[2];
                printf("Proximity Event Data -> Val1: %d, Val2: %d", val1, val2);
            }
            break;
        case 0x0027: // Proximity Sensor Reading (high-detail)
             if (msg->data_len >= 4) {
                int16_t val1 = (msg->data[1] << 8) | msg->data[0];
                int16_t val2 = (msg->data[3] << 8) | msg->data[2];
                if (val1 == 0 && val2 == 0) printf("Proximity Sensor -> State: Idle");
                else printf("Proximity Sensor -> Event Detected (vals: %d, %d)", val1, val2);
            }
            break;
        case 0x0038: 
            printf("Buffered Unknown Sensor Data (0x0038)"); 
            break;
        case 0x002f: // RESP_END_OF_BUFFERS
            printf("End of Buffers"); 
            break;
        default:
            printf(">>> UNKNOWN COMMAND <<< DATA: ");
            print_hex(msg->data, msg->data_len > 32 ? 32 : msg->data_len);
            if (msg->data_len > 32) printf("...");
            break;
    }
    printf("\n");
}


int send_message(int fd, uint16_t command, const uint8_t* data, uint16_t data_len) {
    uint8_t buffer[MAX_RAW_MSG_LEN];
    buffer[0] = 0xd5; buffer[1] = 0xaa; buffer[2] = 0x96;
    buffer[3] = command & 0xFF;
    buffer[4] = (command >> 8) & 0xFF;
    buffer[5] = data_len & 0xFF;
    buffer[6] = (data_len >> 8) & 0xFF;

    if (data != NULL && data_len > 0) memcpy(&buffer[7], data, data_len);

    uint8_t temp_buf_for_crc[MAX_DATA_LEN + 4];
    memcpy(temp_buf_for_crc, &buffer[3], 4 + data_len);
    uint16_t crc = crc16ccitt(temp_buf_for_crc, 4 + data_len);
    buffer[7 + data_len] = crc & 0xFF;
    buffer[8 + data_len] = (crc >> 8) & 0xFF;

    size_t msg_len = 9 + data_len;
    printf("%s SEND (CMD 0x%04x)\n", get_timestamp(), command);
    if (write(fd, buffer, msg_len) != msg_len) { 
        perror("write"); 
        return -1; 
    }
    return 0;
}

BackplateMessage* parse_byte(uint8_t byte) {
    static ParserState state = PARSER_STATE_WAIT_D5_1;
    static BackplateMessage current_msg;
    static uint16_t data_idx = 0;
    static uint8_t crc_buf[MAX_RAW_MSG_LEN];
    static uint16_t crc_idx = 0;

    switch (state) {
        case PARSER_STATE_WAIT_D5_1: if (byte == 0xd5) state = PARSER_STATE_WAIT_D5_2; break;
        case PARSER_STATE_WAIT_D5_2: state = (byte == 0xd5) ? PARSER_STATE_WAIT_AA : PARSER_STATE_WAIT_D5_1; break;
        case PARSER_STATE_WAIT_AA: state = (byte == 0xaa) ? PARSER_STATE_WAIT_96 : PARSER_STATE_WAIT_D5_1; break;
        case PARSER_STATE_WAIT_96:
            if (byte == 0x96) {
                memset(&current_msg, 0, sizeof(BackplateMessage));
                data_idx = 0; crc_idx = 0;
                state = PARSER_STATE_READ_CMD_L;
            } else state = PARSER_STATE_WAIT_D5_1;
            break;
        case PARSER_STATE_READ_CMD_L: crc_buf[crc_idx++] = byte; current_msg.command = byte; state = PARSER_STATE_READ_CMD_H; break;
        case PARSER_STATE_READ_CMD_H: crc_buf[crc_idx++] = byte; current_msg.command |= (byte << 8); state = PARSER_STATE_READ_LEN_L; break;
        case PARSER_STATE_READ_LEN_L: crc_buf[crc_idx++] = byte; current_msg.data_len = byte; state = PARSER_STATE_READ_LEN_H; break;
        case PARSER_STATE_READ_LEN_H:
            crc_buf[crc_idx++] = byte; current_msg.data_len |= (byte << 8);
            if (current_msg.data_len > MAX_DATA_LEN) state = PARSER_STATE_WAIT_D5_1;
            else if (current_msg.data_len == 0) state = PARSER_STATE_READ_CRC_L;
            else state = PARSER_STATE_READ_DATA;
            break;
        case PARSER_STATE_READ_DATA:
            crc_buf[crc_idx++] = byte; current_msg.data[data_idx++] = byte;
            if (data_idx == current_msg.data_len) state = PARSER_STATE_READ_CRC_L;
            break;
        case PARSER_STATE_READ_CRC_L: current_msg.crc = byte; state = PARSER_STATE_READ_CRC_H; break;
        case PARSER_STATE_READ_CRC_H:
            current_msg.crc |= (byte << 8);
            uint16_t calculated_crc = crc16ccitt(crc_buf, 4 + current_msg.data_len);
            current_msg.crc_ok = (calculated_crc == current_msg.crc);
            state = PARSER_STATE_WAIT_D5_1;
            return &current_msg;
    }
    return NULL;
}

int set_interface_attribs(int fd, int speed) {
    struct termios tty;
    if (tcgetattr(fd, &tty) != 0) { perror("tcgetattr"); return -1; }
    cfsetospeed(&tty, speed); cfsetispeed(&tty, speed);
    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
    tty.c_iflag &= ~IGNBRK; tty.c_lflag = 0; tty.c_oflag = 0;
    tty.c_cc[VMIN]  = 0; tty.c_cc[VTIME] = 0; // Set to non-blocking
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON | IXOFF | IXANY);
    tty.c_cflag |= (CLOCAL | CREAD);
    tty.c_cflag &= ~(PARENB | PARODD | CSTOPB | CRTSCTS);
    tty.c_lflag &= ~(ISIG | ICANON | ECHO | ECHOE | ECHOK | ECHONL | IEXTEN);
    tty.c_oflag &= ~OPOST;
    if (tcsetattr(fd, TCSANOW, &tty) != 0) { perror("tcsetattr"); return -1; }
    return 0;
}

void set_screen_color(int screen_buffer, uint32_t color) {
   if (screen_buffer < 0) return;
   struct fb_var_screeninfo vinfo;
   struct fb_fix_screeninfo finfo;
   if (ioctl(screen_buffer, FBIOGET_VSCREENINFO, &vinfo) == -1) { perror("Error reading variable screen info"); return; }
   if (ioctl(screen_buffer, FBIOGET_FSCREENINFO, &finfo) == -1) { perror("Error reading fixed screen info"); return; }
   long screensize = vinfo.yres_virtual * finfo.line_length;
   char *fbp = (char*)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, screen_buffer, 0);
   if (fbp == MAP_FAILED) { perror("Error mapping framebuffer to memory"); return; }
   uint32_t *pixel = (uint32_t*)fbp;
   for (long i = 0; i < (screensize / 4); i++) pixel[i] = color;
   munmap(fbp, screensize);
}

void update_screen_from_als(uint16_t lux_value) {
    if (g_screen_fd < 0) return;
    const float lux_min = 500.0, lux_max = 10000.0;
    const float green_min = 0.0, green_max = 255.0;
    uint8_t green_val = 0;
    if (lux_value <= lux_min) green_val = (uint8_t)green_min;
    else if (lux_value >= lux_max) green_val = (uint8_t)green_max;
    else green_val = (uint8_t)(green_min + ((lux_value - lux_min) / (lux_max - lux_min)) * (green_max - green_min));
    uint32_t color = (uint32_t)green_val << 8;
    printf(" -> Screen Update: Lux=%u -> Green=%d", lux_value, green_val);
    set_screen_color(g_screen_fd, color);
}


// --- Forward declarations for main logic ---
BackplateMessage* read_and_parse_message(int fd, int timeout_ms);
bool get_info(int fd, uint16_t cmd_to_send, uint16_t expected_resp);

int main(int argc, char *argv[]) {
    if (argc > 1 && strcmp(argv[1], "--raw") == 0) { g_print_raw = true; printf("Raw byte printing enabled.\n"); }

    signal(SIGINT, signal_handler);
    init_crctab();
    printf("Cuckoo Talk: Starting backplate communication (v14 - Correct Handshake & Polling)\n");

    g_screen_fd = open(FB_DEVICE_PATH, O_RDWR);
    if (g_screen_fd < 0) perror("Failed to open screen buffer");
    else { printf("Successfully opened screen buffer. Setting to black.\n"); set_screen_color(g_screen_fd, 0x000000); }

    int fd = open(DEVICE_PATH, O_RDWR | O_NOCTTY);
    if (fd < 0) { perror("Error opening device"); return 1; }
    printf("Successfully opened %s\n", DEVICE_PATH);

    set_interface_attribs(fd, B115200);
    tcflush(fd, TCIOFLUSH);
    printf("Sending break...\n"); 
    tcsendbreak(fd, 0); 
    usleep(250000); 
    tcflush(fd, TCIOFLUSH);
    printf("Device configured. Ready.\n--------------------------------------------\n");

    // === PHASE 1: INITIATION AND BURST ===
    printf("\n--- Phase 1: Sending Reset and waiting for 'BRK' ---\n");
    send_message(fd, 0x00ff, NULL, 0);

    uint8_t fet_presence_data[32];
    uint16_t fet_presence_len = 0;
    bool fet_data_received = false;
    bool burst_complete = false;
    time_t start_time = time(NULL);

    while (time(NULL) - start_time < 5 && !burst_complete) {
        BackplateMessage* msg = read_and_parse_message(fd, 500);
        if (msg) {
            if (msg->command == 0x0004 && msg->data_len < 32) {
                memcpy(fet_presence_data, msg->data, msg->data_len);
                fet_presence_len = msg->data_len;
                fet_data_received = true;
            }
            if (msg->command == 0x0001 && msg->data_len == 3 && memcmp(msg->data, "BRK", 3) == 0) {
                burst_complete = true;
            }
        }
    }
    if (!burst_complete) { fprintf(stderr, "Handshake Error: Did not receive 'BRK' signal.\n"); close(fd); return 1; }
    if (!fet_data_received) { fprintf(stderr, "Handshake Error: Did not receive FET presence data.\n"); close(fd); return 1; }
    printf("--- Phase 1 Complete ---\n");

    // === PHASE 2: INFO GATHERING ===
    printf("\n--- Phase 2: Sending ACKs and Gathering Info ---\n");
    send_message(fd, 0x008f, fet_presence_data, fet_presence_len); usleep(50000);
    if (!get_info(fd, 0x0098, 0x0018)) return 1; usleep(50000);
    if (!get_info(fd, 0x0099, 0x0019)) return 1; usleep(50000);
    if (!get_info(fd, 0x009d, 0x001d)) return 1;
    printf("--- Phase 2 Complete ---\n");

    // === PHASE 3: SET OPERATIONAL MODE ===
    printf("\n--- Phase 3: Setting Power Steal Mode ---\n");
    uint8_t power_steal_payload[] = {0, 0, 0, 0};
    send_message(fd, 0x00c0, power_steal_payload, 4);
    sleep(1);
    printf("--- Phase 3 Complete ---\n");

    // === PHASE 4: MAIN POLLING LOOP ===
    printf("\n--- Phase 4: Entering Main Polling Loop (Ctrl+C to exit) ---\n");
    time_t last_poll_time = 0;
    time_t last_keepalive_time = 0;

    fcntl(fd, F_SETFL, O_NONBLOCK); // Switch to non-blocking for the loop

    while (g_running) {
        time_t current_time = time(NULL);

        // A. Send periodic commands
        if (last_keepalive_time == 0 || current_time - last_keepalive_time >= 15) {
             send_message(fd, 0x0083, NULL, 0); // GET_STATUS keep-alive
             last_keepalive_time = current_time;
        }
        if (last_poll_time == 0 || current_time - last_poll_time >= 30) {
            printf("\n%s --- Requesting Historical Data Buffers ---\n", get_timestamp());
            send_message(fd, 0x00a2, NULL, 0); // REQ_HISTORICAL_DATA
            last_poll_time = current_time;
        }
        
        // B. Read all available data
        uint8_t read_buf;
        ssize_t bytes_read;
        do {
            bytes_read = read(fd, &read_buf, 1);
            if (bytes_read > 0) {
                BackplateMessage* msg = parse_byte(read_buf);
                if (msg) {
                    process_message(msg);
                    if (msg->command == 0x002f) { // End of Buffers
                         send_message(fd, 0x00a3, NULL, 0); // ACK End of Buffers
                    }
                }
            }
        } while (bytes_read > 0);
        
        // C. Sleep to prevent busy-looping
        usleep(20000); // 20ms sleep
    }

    // --- Cleanup ---
    if (g_screen_fd >= 0) {
        printf("Setting screen to black before exit.\n");
        set_screen_color(g_screen_fd, 0x000000);
        close(g_screen_fd);
    }
    close(fd);
    printf("\n--------------------------------------------\n");
    printf("Cuckoo Talk finished.\n");
    return 0;
}

BackplateMessage* read_and_parse_message(int fd, int timeout_ms) {
    uint8_t read_buf;
    struct timeval start, now;
    gettimeofday(&start, NULL);

    while (g_running) {
        gettimeofday(&now, NULL);
        long elapsed_ms = (now.tv_sec - start.tv_sec) * 1000 + (now.tv_usec - start.tv_usec) / 1000;
        if (elapsed_ms > timeout_ms) {
            return NULL; // Timeout
        }

        if (read(fd, &read_buf, 1) > 0) {
            BackplateMessage* msg = parse_byte(read_buf);
            if (msg) {
                process_message(msg);
                return msg;
            }
        } else {
            usleep(1000); // Small delay to prevent busy-waiting
        }
    }
    return NULL;
}

bool get_info(int fd, uint16_t cmd_to_send, uint16_t expected_resp) {
    send_message(fd, cmd_to_send, NULL, 0);
    time_t start = time(NULL);
    while(time(NULL) - start < 2) { // 2 second timeout
        BackplateMessage* msg = read_and_parse_message(fd, 200);
        if (msg && msg->command == expected_resp) {
            return true;
        }
    }
    fprintf(stderr, "Timeout waiting for response 0x%04x to command 0x%04x\n", expected_resp, cmd_to_send);
    return false;
}

