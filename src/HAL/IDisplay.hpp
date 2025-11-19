#pragma once
#include <string>
#include <stdint.h>

enum class Font {
    FONT_H1,
    FONT_H2,
    FONT_DEFAULT
};

class IDisplay {
public:
    virtual ~IDisplay() = default;
    virtual bool Initialize() = 0;
    virtual void SetBackgroundColor(uint32_t color) = 0;
    virtual void DrawText(int x, int y, const std::string &text, uint32_t color = 0xFFFFFF, Font font = Font::FONT_DEFAULT) = 0;
    virtual void TimerHandler() = 0;
    virtual void DrawArc(int x, int y, int radius, int start_angle, int end_angle, uint32_t color, int thickness, uint32_t background_color = 0x202020, bool rounded = true) = 0;
};
