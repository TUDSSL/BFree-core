#ifndef EPD_TEMP_H_
#define EPD_TEMP_H_

#include <stdint.h>

void epd_font_show(int16_t line, int16_t idx, uint8_t c);
void epd_init_temp(void);
void epd_draw_base_temp(void);
void epd_draw_temp(uint8_t temp);

#endif /* EPD_TEMP_H_ */
