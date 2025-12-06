#pragma once
#include <stdint.h>

// 8x8 bitmap font for ASCII characters 0-127
// Each character is 8 pixels wide and 8 pixels tall
// Each byte represents one row, with bits 7-0 corresponding to pixels left-to-right

namespace BitmapFont {

// Font dimensions
constexpr int CHAR_HEIGHT = 8;
constexpr int FONT_START_CHAR = 0;
constexpr int FONT_END_CHAR = 127;
constexpr int FONT_CHAR_COUNT = FONT_END_CHAR - FONT_START_CHAR + 1;

// Font data - 8 bytes per character, 128 characters (ASCII 0-127)
extern const uint8_t font_data[FONT_CHAR_COUNT][CHAR_HEIGHT];

} // namespace BitmapFont