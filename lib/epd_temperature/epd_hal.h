#ifndef EPD_COMM_H_
#define EPD_COMM_H_

#include "hal/include/hal_gpio.h"
#include "hal/include/hal_delay.h"
#include "atmel_start_pins.h"
#include "instance/sercom2.h"
#include "shared-bindings/busio/SPI.h"
#include "shared-bindings/digitalio/DigitalInOut.h"
#include "shared-bindings/microcontroller/Pin.h"
#include "py/mpconfig.h"

// Yellow SCK
// Blue MOSI

#define EPD_BUSY_PIN        (&pin_PA11) // D0 - Busy pin
#define EPD_CS_PIN          (&pin_PA10) // D1 - Chip select pin
#define EPD_DC_PIN          (&pin_PA14) // D2 - Data/Command
#define EPD_RST_PIN         (&pin_PA09) // D3 - Reset

#define EPD_SPI_SCK_PIN     (&pin_PB11) // SCK header
#define EPD_SPI_MOSI_PIN    (&pin_PB10) // MOSI header
#define EPD_SPI_MISO_PIN    NULL        // None (EPD has not data out)
#define EPD_SPI_FREQ        (4000000)   // 4MHz

/*
 * EPD communication
 */
struct spi_m_sync_descriptor SPI_M_SERCOM2;
busio_spi_obj_t epd_spi_bus;

digitalio_digitalinout_obj_t epd_cs_pin;
digitalio_digitalinout_obj_t epd_rst_pin;
digitalio_digitalinout_obj_t epd_busy_pin;
digitalio_digitalinout_obj_t epd_dc_pin;

bool epd_performed_comm_init = false;

extern void _common_hal_busio_spi_construct(busio_spi_obj_t *self,
        const mcu_pin_obj_t * clock, const mcu_pin_obj_t * mosi,
        const mcu_pin_obj_t * miso);


bool epd_performed_one_time_construct = false; // In checkpoint
static void epd_one_time_construct(void) {

    if (epd_performed_one_time_construct == true) {
        return;
    }

    // Pin base
    epd_cs_pin.base.type = &digitalio_digitalinout_type;
    epd_rst_pin.base.type = &digitalio_digitalinout_type;
    epd_busy_pin.base.type = &digitalio_digitalinout_type;
    epd_dc_pin.base.type = &digitalio_digitalinout_type;

    // CS pin
    common_hal_digitalio_digitalinout_construct(&epd_cs_pin, EPD_CS_PIN);
    common_hal_digitalio_digitalinout_switch_to_output(&epd_cs_pin, true, DRIVE_MODE_PUSH_PULL);
    common_hal_digitalio_digitalinout_never_reset(&epd_cs_pin);
    common_hal_digitalio_digitalinout_set_value(&epd_cs_pin, true);

    // RST pin
    common_hal_digitalio_digitalinout_construct(&epd_rst_pin, EPD_RST_PIN);
    common_hal_digitalio_digitalinout_switch_to_output(&epd_rst_pin, true, DRIVE_MODE_PUSH_PULL);
    common_hal_digitalio_digitalinout_never_reset(&epd_rst_pin);

    // Data/Command pin
    common_hal_digitalio_digitalinout_construct(&epd_dc_pin, EPD_DC_PIN);
    common_hal_digitalio_digitalinout_switch_to_output(&epd_dc_pin, true, DRIVE_MODE_PUSH_PULL);
    common_hal_digitalio_digitalinout_never_reset(&epd_dc_pin);

    // BUSY pin (INPUT)
    common_hal_digitalio_digitalinout_construct(&epd_busy_pin, EPD_BUSY_PIN);
    common_hal_digitalio_digitalinout_switch_to_input(&epd_busy_pin, PULL_NONE);
    common_hal_digitalio_digitalinout_never_reset(&epd_busy_pin);

    // SPI
    epd_spi_bus.base.type = &busio_spi_type;
    epd_spi_bus.spi_desc = SPI_M_SERCOM2;
    common_hal_busio_spi_construct(&epd_spi_bus, EPD_SPI_SCK_PIN, EPD_SPI_MOSI_PIN, EPD_SPI_MISO_PIN);
    common_hal_busio_spi_configure(&epd_spi_bus, EPD_SPI_FREQ, 0, 0, 8);
    //common_hal_busio_spi_never_reset(&epd_spi_bus);

    epd_performed_one_time_construct = true;
}

static void epd_comm_init(void) {

    if (epd_performed_comm_init == true)
        return;

    if (!epd_performed_one_time_construct) {
        epd_one_time_construct();
    }

    // SPI
    //epd_spi_bus.base.type = &busio_spi_type;
    //epd_spi_bus.spi_desc = SPI_M_SERCOM2;
    //_common_hal_busio_spi_construct(&epd_spi_bus, EPD_SPI_SCK_PIN, EPD_SPI_MOSI_PIN, EPD_SPI_MISO_PIN);
    common_hal_busio_spi_configure(&epd_spi_bus, EPD_SPI_FREQ, 0, 0, 8);
    //common_hal_busio_spi_never_reset(&epd_spi_bus);

    epd_performed_comm_init = true;
}

static inline void epd_spi_cs_low(void) {
    common_hal_digitalio_digitalinout_set_value(&epd_cs_pin, false);
}

static inline void epd_spi_cs_high(void) {
    common_hal_digitalio_digitalinout_set_value(&epd_cs_pin, true);
}

static inline void epd_dc_low(void) {
    common_hal_digitalio_digitalinout_set_value(&epd_dc_pin, false);
}

static inline void epd_dc_high(void) {
    common_hal_digitalio_digitalinout_set_value(&epd_dc_pin, true);
}

static inline void epd_rst_low(void) {
    common_hal_digitalio_digitalinout_set_value(&epd_rst_pin, false);
}

static inline void epd_rst_high(void) {
    common_hal_digitalio_digitalinout_set_value(&epd_rst_pin, true);
}

static inline void epd_spi_write(uint8_t data) {
    //while (!common_hal_busio_spi_try_lock(&epd_spi_bus));
    common_hal_busio_spi_write(&epd_spi_bus, (uint8_t *)&data, 1);
}

static inline void epd_delay_ms(uint32_t ms) {
    delay_ms(ms);
}

static inline void epd_reset(void) {
    epd_rst_high();
    epd_rst_low();

    //epd_delay_ms(10); // TODO: Why does delay act strange?
    for (int i=0; i<50000; i++) {
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
    }
    epd_rst_high();
}

void epd_wait_busy(void) {
    while (1) {
        if (common_hal_digitalio_digitalinout_get_value(&epd_busy_pin) == false)
            break;
    }
}

#endif /* EPD_COMM_H_ */
