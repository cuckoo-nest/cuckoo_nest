#pragma once
#include <string>
#include <stdint.h>
#include <linux/fb.h>

class Display {
public:
    Display(std::string device_path);
    ~Display();
    bool initialize();
    void SetBackgroundColor(uint32_t color);
    void DrawText(int x, int y, const std::string &text, uint32_t color = 0xFFFFFF, int scale = 1);
    void DrawLine(int x0, int y0, int x1, int y1, uint32_t color);
    void DrawPixel(int x, int y, uint32_t color);
    void Flush();

private:
    std::string device_path_;
    int screen_buffer;
    char *fbp;
    char *working_buffer;
    long screensize;
    
    // Screen info
    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;
    
    // Helper functions
    void DrawBitmapChar(char c, int x, int y, uint32_t color, int scale);
};