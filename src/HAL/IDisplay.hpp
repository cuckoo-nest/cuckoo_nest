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
    virtual bool Initialize(bool emulate) = 0;
    virtual void SetBackgroundColor(uint32_t color) = 0;
    virtual void DrawText(int x, int y, const std::string &text, uint32_t color = 0xFFFFFF, Font font = Font::FONT_DEFAULT) = 0;
    virtual void TimerHandler() = 0;

    inline int width() const { return res_w_; };
    inline int height() const { return res_h_; };
protected:
	int res_w_ = 0;
	int res_h_ = 0;
};
