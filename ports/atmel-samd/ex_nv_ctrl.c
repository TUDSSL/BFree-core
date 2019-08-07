#include "ex_nv_ctrl.h"
#include "sercom.h"

struct spi_m_sync_descriptor SPI_M_SERCOM0;

void _init() {
    

}

uint8_t write_bytes_to_nv(uint8_t * arr, uint16_t addr) {

}

uint8_t read_bytes_from_nv(uint16_t addr, uint16_t len, uint8_t * target_arr) {

}

uint8_t get_last_checkpoint(uint8_t * target_mem) {

}

uint8_t checkpoint_exists() {

}