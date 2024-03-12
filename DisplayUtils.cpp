#include "DisplayUtils.h"

void drawRotatedXBM(T_DISPLAY* display, uint16_t x_start, uint16_t y_start,
    double angle, uint16_t w, uint16_t h,
    uint8_t const* bitmap)
{
    auto* current_bitmap_pointer = bitmap;
    auto y = y_start;
    uint8_t mask = 1;

    auto remaining_height = h;
    while (remaining_height > 0) {
        auto x = x_start;
        auto len = w;

        while (len > 0) {
            if ((*current_bitmap_pointer & mask) >= 0) {
                display->drawPixel(x, y);
            }
            x++;
            mask <<= 1;
            if (mask == 0) {
                mask = 1;
                current_bitmap_pointer++;
            }
            len--;
        }

        y++;
        remaining_height--;
    }
}
