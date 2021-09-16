/*
 * Port of https://github.com/wemos/LOLIN_EPD_Library
 */

#include <stdint.h>
#include <stdbool.h>

#include "epd.h"

// One-time includes that create instances
// Should not be included directly in any other file
#include "epd_hal.h"
#include "epd_lut.h"

#define EPD_HEIGHT          (122)
#define EPD_HEIGHT_ALIGNED  (128)
#define EPD_WIDTH           (250)
#define EPD_WIDTH_ALIGNED   (256)
#define EPD_SCREEN_BYTES    (EPD_HEIGHT_ALIGNED*EPD_WIDTH_ALIGNED/8)

void epd_send_command(uint8_t command) {
    epd_spi_cs_high();
    epd_dc_low();
    epd_spi_cs_low();

    epd_spi_write(command);

    epd_spi_cs_high();
}

void epd_send_data(uint8_t data) {
    epd_spi_cs_high();
    epd_dc_high();
    epd_spi_cs_low();

    epd_spi_write(data);

    epd_spi_cs_high();
}

void epd_deep_sleep(void) {
  epd_send_command(0x10);
  epd_send_data(0x01);
}

void epd_select_lut(uint8_t *lut) {
    epd_send_command(0x32);
    for (int count = 0; count < 70; count++)
        epd_send_data(lut[count]);
}

void epd_init(bool init_comm) {
    if (init_comm) {
        epd_comm_init();
    }

    epd_reset();

    epd_wait_busy();
    epd_send_command(0x12); // soft-reset
    epd_wait_busy();

    epd_send_command(0x74); //set analog block control
    epd_send_data(0x54);
    epd_send_command(0x7E); //set digital block control
    epd_send_data(0x3B);

    epd_send_command(0x01); //Driver output control
    epd_send_data(0xF9);
    epd_send_data(0x00);
    epd_send_data(0x00);

    epd_send_command(0x11); //data entry mode
    epd_send_data(0x01);

    epd_send_command(0x44); //set Ram-X address start/end position
    epd_send_data(0x00);
    epd_send_data(0x0F); //0x0C-->(15+1)*8=128

    epd_send_command(0x45);  //set Ram-Y address start/end position
    epd_send_data(0xF9); //0xF9-->(249+1)=250
    epd_send_data(0x00);
    epd_send_data(0x00);
    epd_send_data(0x00);

    epd_send_command(0x3C); //BorderWavefrom
    epd_send_data(0x03);

    epd_send_command(0x2C);  //VCOM Voltage
    epd_send_data(0x55); //

    epd_send_command(0x03); //
    epd_send_data(LUT_DATA[70]);

    epd_send_command(0x04); //
    epd_send_data(LUT_DATA[71]);
    epd_send_data(LUT_DATA[72]);
    epd_send_data(LUT_DATA[73]);

    epd_send_command(0x3A); //Dummy Line
    epd_send_data(LUT_DATA[74]);
    epd_send_command(0x3B); //Gate time
    epd_send_data(LUT_DATA[75]);

    epd_select_lut((unsigned char *)LUT_DATA); //LUT

    epd_send_command(0x4E); // set RAM x address count to 0;
    epd_send_data(0x00);
    epd_send_command(0x4F); // set RAM y address count to 0X127;
    epd_send_data(0xF9);
    epd_send_data(0x00);
    epd_wait_busy();
}

void epd_update(void) {
    epd_send_command(0x22);
    epd_send_data(0xC7);
    epd_send_command(0x20);

    epd_wait_busy();
}

void epd_fill(uint8_t c) {
    epd_send_command(0x24);

    for (uint16_t i=0; i<EPD_SCREEN_BYTES; i++) {
        epd_send_data(c);
    }

    epd_update();
}

void epd_partial_init(void) {
    epd_send_command(0x2C); //VCOM Voltage
    epd_send_data(0x26);

    epd_wait_busy();
    epd_select_lut((unsigned char *)LUT_DATA_part);

    epd_send_command(0x37);
    epd_send_data(0x00);
    epd_send_data(0x00);
    epd_send_data(0x00);
    epd_send_data(0x00);
    epd_send_data(0x40);
    epd_send_data(0x00);
    epd_send_data(0x00);

    epd_send_command(0x22);
    epd_send_data(0xC0);
    epd_send_command(0x20);
    epd_wait_busy();

    epd_send_command(0x3C); //BorderWavefrom
    epd_send_data(0x01);
}

void epd_partial_update(void) {
    epd_send_command(0x22);
    epd_send_data(0x0C);
    epd_send_command(0x20);
    epd_wait_busy();
}

// Why the 0x26? 0x24 is filling RAM
#define EPD_UPDATE_AFTER_PARTIAL_BASE_FILL (0)
void epd_partial_base_fill(uint8_t c) {
    epd_send_command(0x24); ////Write Black and White image to RAM
    for (uint16_t i=0; i<EPD_SCREEN_BYTES; i++) {
        epd_send_data(c);
    }

    epd_send_command(0x26); ////Write Black and White image to RAM
    for (uint16_t i=0; i<EPD_SCREEN_BYTES; i++) {
        epd_send_data(c);
    }

    #if EPD_UPDATE_AFTER_PARTIAL_BASE_FILL
    epd_update();
    #endif
}

void epd_partial_display_prepare(int16_t x_start, int16_t y_start, int16_t PART_COLUMN, int16_t PART_LINE) {
    int16_t x_end, y_start1, y_start2, y_end1, y_end2;
    x_start = x_start / 8; //
    x_end = x_start + PART_LINE / 8 - 1;

    y_start1 = 0;
    y_start2 = y_start;
    if (y_start >= 256) {
        y_start1 = y_start2 / 256;
        y_start2 = y_start2 % 256;
    }
    y_end1 = 0;
    y_end2 = y_start + PART_COLUMN - 1;
    if (y_end2 >= 256) {
        y_end1 = y_end2 / 256;
        y_end2 = y_end2 % 256;
    }

    epd_send_command(0x44);     // set RAM x address start/end, in page 35
    epd_send_data(x_start);     // RAM x address start at 00h;
    epd_send_data(x_end);       // RAM x address end at 0fh(15+1)*8->128
    epd_send_command(0x45);     // set RAM y address start/end, in page 35
    epd_send_data(y_start2);    // RAM y address start at 0127h;
    epd_send_data(y_start1);    // RAM y address start at 0127h;
    epd_send_data(y_end2);      // RAM y address end at 00h;
    epd_send_data(y_end1);      // ????=0

    epd_send_command(0x4E);     // set RAM x address count to 0;
    epd_send_data(x_start);
    epd_send_command(0x4F);     // set RAM y address count to 0X127;
    epd_send_data(y_start2);
    epd_send_data(y_start1);
}

void epd_partial_display(int16_t x_start, int16_t y_start, int16_t PART_COLUMN, int16_t PART_LINE, const unsigned char *data) {
    epd_partial_display_prepare(x_start, y_start, PART_COLUMN, PART_LINE);

    epd_send_command(0x24); //Write Black and White image to RAM
    for (uint16_t i = 0; i < PART_COLUMN * PART_LINE / 8; i++) {
        uint8_t b = data[i];
        epd_send_data(b);
    }
    epd_partial_update();
}

void epd_partial_display_fill(int16_t x_start, int16_t y_start, int16_t PART_COLUMN, int16_t PART_LINE, uint8_t c) {
    epd_partial_display_prepare(x_start, y_start, PART_COLUMN, PART_LINE);

    epd_send_command(0x24); //Write Black and White image to RAM
    for (uint16_t i = 0; i < PART_COLUMN * PART_LINE / 8; i++)
    {
        epd_send_data(c);
    }
    epd_partial_update();
}

void epd_partial_clear(void) {
    epd_partial_display_fill(0, 256, 256, 128, 0xFF);
    epd_partial_display_fill(0, 256, 256, 128, 0x00);
}
