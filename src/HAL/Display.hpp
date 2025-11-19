#pragma once
#include <string>
#include <stdint.h>
#include <linux/fb.h>

#include "lvgl/lvgl.h"
#include "IDisplay.hpp"

class Display : public IDisplay {
public:
    Display(std::string device_path);
    ~Display() override;
    bool Initialize() override;
    void SetBackgroundColor(uint32_t color) override;
    void DrawText(int x, int y, const std::string &text, uint32_t color = 0xFFFFFF, Font font = Font::FONT_DEFAULT) override;
    void TimerHandler() override;
    void DrawArc(int x, int y, int radius, int start_angle, int end_angle, uint32_t color, int thickness, uint32_t background_color = 0x202020, bool rounded = true);

private:
    std::string device_path_;
    int screen_buffer;
    char *fbp;
    char *working_buffer;
    long screensize;
    
    // Screen info
    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;

    // lvgl display members
    lv_display_t *disp;
    lv_style_t *fontH2;
    lv_style_t *fontH1;
};
