/**
 * API for sennding and recieving bytes to/from the external Non-volatile memory controller,
 * which is a MSP430FR59941 in the current version.
 * 
 * PIN ASSIGNMENTS  for the Interface to the MSP430.
 * ===============================================
 * CP  |     Function    |  MSP |  M0
 * ===============================================
 * D13 | CLK             | P5.2 | PA17/SERCOM1+3.1
 * D12 | SOMI            | P5.1 | PA19/SERCOM1+3.3
 * D11 | SIMO            | P5.0 | PA16/SERCOM1+3.0
 * D10 | CS              | P5.3 | PA18/SERCOM1+3.2
 * D9  | NV_WRITE / GPIO1| P1.7 | PA07
 * D8  | VCAP_HALF / ADC | P7.4 | PA06
 * D7  | NV_READ         | P1.6 | PA21
 * D6  | PFAIL           | P2.0 | PA20
 * 
 * @author Josiah Hester
 * 
 */
#ifndef EX_NV_CTRL_H
#define EX_NV_CTRL_H

#include <stdint.h>

void _init_nv_interface();

uint8_t write_bytes_to_nv(uint8_t * arr, uint16_t addr);
uint8_t read_bytes_from_nv(uint16_t addr, uint16_t len, uint8_t * target_arr);
uint8_t get_last_checkpoint(uint8_t * target_mem);
uint8_t checkpoint_exists();

#endif |