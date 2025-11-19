#include "Display.hpp"
#include "BitmapFont.hpp"
#include "../Screens/CuckooLogoNest.hpp"
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

bool Display::Initialize()
{
    lv_init();

    //Create a display
    disp = lv_linux_fbdev_create();
    if (disp == NULL) {
        perror("lv_linux_fbdev_create failed");
        return false;
    }

    lv_linux_fbdev_set_file(disp, "/dev/fb0");
    lv_display_set_resolution(disp, 320, 320);

    // Setup fonts
    fontH1 = new lv_style_t;
    lv_style_init(fontH1);
    lv_style_set_text_font(fontH1, &lv_font_montserrat_48);

    fontH2 = new lv_style_t;
    lv_style_init(fontH2);
    lv_style_set_text_font(fontH2, &lv_font_montserrat_28);

    // Display logo image
    // Extract background color from first pixel of image (RGB565 format)
    uint16_t first_pixel = (CuckooLogoNest.data[1] << 8) | CuckooLogoNest.data[0];
    uint32_t bg_color = ((first_pixel & 0xF800) << 8) |  // Red (5 bits -> 8 bits)
                        ((first_pixel & 0x07E0) << 5) |  // Green (6 bits -> 8 bits)
                        ((first_pixel & 0x001F) << 3);   // Blue (5 bits -> 8 bits)
    
    SetBackgroundColor(bg_color);
    lv_obj_t * img1 = lv_image_create(lv_screen_active());
    lv_image_set_src(img1, &CuckooLogoNest);
    lv_obj_align(img1, LV_ALIGN_CENTER, 0, 0);
    // Force a render
    lv_timer_handler();
    
    sleep(3);

    return true;
}

void Display::SetBackgroundColor(uint32_t color)
{
    lv_obj_t * scr = lv_scr_act();
    lv_obj_clean(scr);
    lv_obj_set_style_bg_color(scr, lv_color_make((color >> 16) & 0xFF, (color >> 8) & 0xFF, color & 0xFF), 0);
}
void Display::DrawText(int x, int y, const std::string &text, uint32_t color, Font font)
{
    lv_obj_t * label = lv_label_create(lv_screen_active());
    lv_label_set_text(label, text.c_str());

    switch (font)
    {
        case Font::FONT_H1:
            lv_obj_add_style(label, fontH1, 0);
            break;

        case Font::FONT_H2:
            lv_obj_add_style(label, fontH2, 0);
            break;

        case Font::FONT_DEFAULT:
        default:
            lv_obj_add_style(label, fontH2, 0);
            break;
    }

    lv_obj_add_style(label, fontH2, 0);
    lv_obj_set_style_text_color(label, lv_color_make((color >> 16) & 0xFF, (color >> 8) & 0xFF, color & 0xFF), 0);
    lv_obj_center(label);
    lv_obj_set_y(label, y);
}
void Display::DrawArc(int x, int y, int radius,
                      int start_angle, int end_angle,
                      uint32_t color, int thickness,
                      uint32_t background_color, bool rounded)
{
    lv_obj_t *arc = lv_arc_create(lv_scr_act());

    // Position and size
    lv_obj_set_size(arc, radius * 2, radius * 2);
    lv_obj_set_pos(arc, x - radius, y - radius);

    // Configure full range
    lv_arc_set_range(arc, 0, 100);
    lv_arc_set_rotation(arc, start_angle);
    lv_arc_set_mode(arc, LV_ARC_MODE_NORMAL);

    // Background track
    if(background_color != 0) {
        lv_obj_set_style_arc_width(arc, thickness, LV_PART_MAIN);
        lv_obj_set_style_arc_color(arc,
            lv_color_make((background_color >> 16) & 0xFF,
                          (background_color >> 8) & 0xFF,
                          background_color & 0xFF),
            LV_PART_MAIN);
        lv_obj_set_style_arc_rounded(arc, rounded ? 1 : 0, LV_PART_MAIN);
    } else {
        lv_obj_set_style_arc_width(arc, 0, LV_PART_MAIN);
    }

    // Indicator (fill)
    lv_obj_set_style_arc_width(arc, thickness, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(arc,
        lv_color_make((color >> 16) & 0xFF,
                      (color >> 8) & 0xFF,
                      color & 0xFF),
        LV_PART_INDICATOR);
    lv_obj_set_style_arc_rounded(arc, rounded ? 1 : 0, LV_PART_INDICATOR);

    // Fill the arc proportionally
    int fill_percent = end_angle - start_angle;
    if(fill_percent < 0) fill_percent += 360; // handle wrap-around
    lv_arc_set_value(arc, fill_percent * 100 / 360);
}
void Display::TimerHandler()
{
    lv_timer_handler();
}
