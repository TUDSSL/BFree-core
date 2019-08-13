/*
 * spi.h
 *
 *  Created on: Jun 25, 2019
 *      Author: abubakar
 */

#ifndef SPI_H_
#define SPI_H_

#include<msp430.h>
#include<stdint.h>

// SPI port 5
#define SPI_MOSI BIT0   // P5.0
#define SPI_MISO BIT1   // P5.1
#define SPI_SCLK BIT2   // P5.2

void spiPinConfig(void);
void spiConfig(void);

uint16_t readSpiShort(uint16_t);
uint32_t readSpiLong(uint32_t);

void writeSpiByte(uint8_t);
void writeSpiShort(uint16_t);
void writeSpiLong(uint32_t);



#endif /* SPI_H_ */
