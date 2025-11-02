#include "Display.hpp"
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <iostream>
#include <unistd.h>

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

    // Map the screen buffer to memory
    fbp = (char *)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, screen_buffer, 0);
    if (fbp == MAP_FAILED)
    {
        std::cerr << "Error mapping framebuffer to memory" << std::endl;
        return false;
    }

    return true;
}

void Display::SetBackgroundColor(uint32_t color)
{

    // Set all pixels to the specified color
    // Assuming 32-bit color depth (4 bytes per pixel)
    uint32_t *pixel = (uint32_t *)fbp;
    long pixel_count = screensize / 4;
    long i;

    for (i = 0; i < pixel_count; i++)
    {
        pixel[i] = color;
    }
}
