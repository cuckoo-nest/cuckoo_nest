#pragma once
#include <string>
#include <stdint.h>
#ifdef BUILD_TARGET_LINUX
#include <linux/fb.h>
#endif

#include "lvgl/lvgl.h"
#include "IDisplay.hpp"

class Display : public IDisplay {
public:
    Display(std::string device_path);
    ~Display() override;
    bool Initialize(bool emulate) override;
    void SetBackgroundColor(uint32_t color) override;
    void DrawText(int x, int y, const std::string &text, uint32_t color = 0xFFFFFF, Font font = Font::FONT_DEFAULT) override;
    void TimerHandler() override;

private:
    std::string device_path_;
    int screen_buffer;
    char *fbp;
	void *working_buffer1;
	void *working_buffer2;
    long screensize;
    
    // Screen info
#ifdef BUILD_TARGET_LINUX
    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;
#endif

    // lvgl display members
    lv_display_t *disp;
    lv_style_t *fontH2;
    lv_style_t *fontH1;
};