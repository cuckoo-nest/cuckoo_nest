#include "Display.hpp"
#include "BitmapFont.hpp"
#include "../Screens/CuckooLogoNest.hpp"
#ifdef BUILD_TARGET_LINUX
#include <linux/fb.h>
#endif
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <iostream>
#include <unistd.h>
#include <cstring>


Display::Display(std::string device_path)
    : device_path_(device_path)
{
	res_w_ = 320;
	res_h_ = 320;
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

bool Display::Initialize(bool emulate)
{
    lv_init();
	lv_display_t *disp = NULL;
    if (emulate)
    {
#ifdef HOST_TOOLCHAIN
        disp = lv_sdl_window_create(res_w_, res_h_);
#else
        std::cerr << "Emulation mode requested but not compiled with SDL support" << std::endl;
        return false;
#endif
    }
    else
    {
#ifdef BUILD_TARGET_LINUX
        //Create a display
        disp = lv_linux_fbdev_create();
        if (disp == NULL) {
            perror("lv_linux_fbdev_create failed");
            return false;
        }

        uint bufsize = (res_w_ * res_h_ * 4); //res_w_ * res_h_ * BYTES_PER_PIXEL;
        working_buffer1 = malloc(bufsize);
        working_buffer2 = malloc(bufsize);
        lv_display_set_buffers(disp, working_buffer1, working_buffer2, bufsize, LV_DISPLAY_RENDER_MODE_PARTIAL);

        lv_linux_fbdev_set_file(disp, "/dev/fb0");
#endif
    }
	lv_display_set_resolution(disp, res_w_, res_h_);

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
    
    #ifdef HOST_TOOLCHAIN
        usleep ( 500 * 1000 );   // 500ms
    #else
        sleep (3);
    #endif

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

void Display::TimerHandler()
{
    lv_timer_handler();
}