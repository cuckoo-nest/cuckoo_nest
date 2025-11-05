#include "Display.hpp"
#include "BitmapFont.hpp"
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <iostream>
#include <unistd.h>
#include <cstring>

Display::Display(std::string device_path) : device_path_(device_path)
{
}

Display::~Display()
{
    if (fbp != nullptr)
    {
        munmap(fbp, screensize);
    }
    if (screen_buffer >= 0)
    {
        close(screen_buffer);
    }
}

bool Display::initialize()
{
    screen_buffer = open("/dev/fb0", O_RDWR);
    if (screen_buffer < 0)
    {
        std::cerr << "Failed to open screen buffer" << std::endl;
        return false;
    }

    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;

    // Get screen info
    if (ioctl(screen_buffer, FBIOGET_VSCREENINFO, &vinfo) == -1)
    {
        std::cerr << "Error reading variable screen info" << std::endl;
        return false;
    }

    if (ioctl(screen_buffer, FBIOGET_FSCREENINFO, &finfo) == -1)
    {
        std::cerr << "Error reading fixed screen info" << std::endl;
        return false;
    }

    std::cout << "Screen resolution: " << vinfo.xres << "x" << vinfo.yres << std::endl;

    // Calculate screen buffer size
    screensize = vinfo.yres_virtual * finfo.line_length;

    std::cout << "Screen buffer size: " << screensize << " bytes" << std::endl;

    working_buffer = new char[screensize];
    if (working_buffer == nullptr)
    {
        std::cerr << "Error allocating working buffer" << std::endl;
        return false;
    }

    // Map the device to memory
    fbp = (char *)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, screen_buffer, 0);
    if (fbp == MAP_FAILED)
    {
        std::cerr << "Failed to map framebuffer device to memory" << std::endl;
        return false;
    }

    return true;
}

void Display::SetBackgroundColor(uint32_t color)
{

    // Set all pixels to the specified color
    // Assuming 32-bit color depth (4 bytes per pixel)
    uint32_t *pixel = (uint32_t *)working_buffer;
    long pixel_count = screensize / 4;
    long i;

    for (i = 0; i < pixel_count; i++)
    {
        pixel[i] = color;
    }
}
void Display::DrawText(int x, int y, const std::string &text, uint32_t color, int scale)
{
    int pen_x = x;
    int pen_y = y;
    
    for (char c : text) {
        // Handle newlines
        if (c == '\n') {
            pen_x = x;
            pen_y += BitmapFont::CHAR_HEIGHT * scale;
            continue;
        }
        
        // Handle carriage returns
        if (c == '\r') {
            pen_x = x;
            continue;
        }
        
        // Draw the character
        DrawBitmapChar(c, pen_x, pen_y, color, scale);
        
        // Advance pen position
        pen_x += BitmapFont::CHAR_WIDTH * scale;
    }
}

void Display::DrawLine(int x0, int y0, int x1, int y1, uint32_t color)
{
    // Bresenham's line algorithm
    int dx = abs(x1 - x0);
    int dy = -abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx + dy; // error value e_xy

    while (true) {
        DrawPixel(x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 >= dy) {
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y0 += sy;
        }
    }
}

void Display::DrawPixel(int x, int y, uint32_t color)
{
    long location = x * 4 + y * 320 * 4;
    *((uint32_t*)(working_buffer + location)) = color;
}

void Display::DrawBitmapChar(char c, int x, int y, uint32_t color, int scale)
{
    // Ensure character is in valid range
    if (c < BitmapFont::FONT_START_CHAR || c > BitmapFont::FONT_END_CHAR) {
        c = '?'; // Replace invalid characters with question mark
    }
    
    // Get font data for this character
    int char_index = c - BitmapFont::FONT_START_CHAR;
    const uint8_t* char_data = BitmapFont::font_data[char_index];
    
    // Draw each pixel of the character
    for (int row = 0; row < BitmapFont::CHAR_HEIGHT; row++) {
        uint8_t row_data = char_data[row];
        
        for (int col = 0; col < BitmapFont::CHAR_WIDTH; col++) {
            // Check if pixel should be drawn (bit 7 = leftmost, bit 0 = rightmost)
            if (row_data & (0x80 >> col)) {
                // Draw scaled pixel(s)
                for (int sy = 0; sy < scale; sy++) {
                    for (int sx = 0; sx < scale; sx++) {
                        DrawPixel(x + col * scale + sx, y + row * scale + sy, color);
                    }
                }
            }
        }
    }
}

void Display::Flush()
{
    // Flush changes to screen
    //ioctl(screen_buffer, FBIOPAN_DISPLAY, &vinfo);
    memcpy(fbp, working_buffer, screensize);
}
