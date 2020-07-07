#include <stdint.h>
#include <stdbool.h>

#include "epd.h"
#include "epd_temp.h"

// One-time includes that create instances
// Should not be included directly in any other file
#include "assets/epd_asset_font.h"
#include "assets/epd_asset_misc.h"

#define FONT_NUM(n_)  (n_)
#define FONT_DEGR_C   10
#define FONT_TEMP     11
static inline const unsigned char *font_get(uint8_t c) {
  return &Font[c * FontBytes];
}

void epd_font_show(int16_t line, int16_t idx, uint8_t c) {
    int16_t x, y;
    x = line * 64;
    y = 256 - (idx * 64);
    epd_partial_display_fill(x, y, 64, 64, 0x00); // clear
    epd_partial_display(x, y, 64, 64, font_get(c)); // show the digit
}

void epd_init_temp(void) {
    epd_init(true);
    epd_partial_base_fill(0xFF);
    //epd_update();
    epd_partial_init();
}

void epd_draw_base_temp(void) {
    epd_init_temp();

    epd_partial_display(64-8, 256-8, 256, 64, BFree);
    epd_font_show(0, 0, FONT_TEMP);
    epd_font_show(0, 3, FONT_DEGR_C);

    epd_deep_sleep();
}

void epd_draw_temp(uint8_t temp) {
    static uint8_t last_unit = 10;
    static uint8_t last_tens = 10;

    epd_init_temp();

    uint8_t unit = temp % 10;
    uint8_t tens = (temp/10) % 10;

    if (unit != last_unit) {
        epd_font_show(0, 2, unit);
    }

    if (tens != last_tens) {
        epd_font_show(0, 1, tens);
    }
    epd_deep_sleep();
}
