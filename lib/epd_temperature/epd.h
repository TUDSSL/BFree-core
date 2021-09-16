#ifndef EPD_H_
#define EPD_H_

#include <stdint.h>
#include <stdbool.h>

void epd_send_command(uint8_t command);
void epd_send_data(uint8_t data);
void epd_deep_sleep(void);
void epd_select_lut(uint8_t *lut);
void epd_init(bool);
void epd_fill(uint8_t c);
void epd_partial_init(void);
void epd_partial_update(void);
void epd_partial_base_fill(uint8_t c);
void epd_partial_display_prepare(int16_t x_start, int16_t y_start, int16_t PART_COLUMN, int16_t PART_LINE);
void epd_partial_display(int16_t x_start, int16_t y_start, int16_t PART_COLUMN, int16_t PART_LINE, const unsigned char *data);
void epd_partial_display_fill(int16_t x_start, int16_t y_start, int16_t PART_COLUMN, int16_t PART_LINE, uint8_t c);
void epd_partial_clear(void);

#endif /* EPD_H_ */
