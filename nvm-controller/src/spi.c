#include "spi.h"

void spiPinConfig(void)
{
    /* Select SPI functionality for the pins */
    P5SEL1 &= ~(SPI_MOSI | SPI_MISO | SPI_SCLK);
    P5SEL0 |= (SPI_MOSI | SPI_MISO | SPI_SCLK);
}

void spiConfig(void)
{
    //Clock Polarity: The inactive state is high
    //MSB First, 8-bit, Master, 3-pin mode, Synchronous
    UCB1CTLW0 = UCSWRST;                       // **Put state machine in
//    UCB1CTLW0 |= UCCKPH | UCMSB | UCSYNC | UCSTEM | UCMODE_2 | UCSSEL__SMCLK;      // 3-pin, 8-bit SPI Slavereset**
    UCB1CTLW0 |= UCCKPH | UCMSB | UCSYNC | UCSTEM | UCMODE_2 | UCSSEL__SMCLK;      // Changed to KPH for Metro M0
//    UCB1BRW = 0xff;
    UCB1CTLW0 &= ~UCSWRST;                     // **Initialize USCI state machine**
    UCB1IE |= UCRXIE;                          // Enable USCI0 RX interrupt
}

uint16_t readSpiShort(uint16_t temp)
{
    uint16_t twoBytes = temp;
    UCB1IE &= ~UCRXIE;                          // Disable USCI0 RX interrupt

    while(!(UCB1IFG & UCRXIFG));
    temp = UCB1RXBUF;
    twoBytes |= (temp << 0x08);

    UCB1IFG &= ~UCRXIFG;
    UCB1IE |= UCRXIE;                           // Enable USCI0 RX interrupt
    return twoBytes;
}

uint32_t readSpiLong(uint32_t temp)
{
    uint32_t fourBytes = temp;
    UCB1IE &= ~UCRXIE;                          // Disable USCI0 RX interrupt

    while(!(UCB1IFG & UCRXIFG));
    temp = UCB1RXBUF;
    fourBytes |= (temp << 8);

    while(!(UCB1IFG & UCRXIFG));
    temp = UCB1RXBUF;
    fourBytes |= (temp << 16);

    while(!(UCB1IFG & UCRXIFG));
    temp = UCB1RXBUF;
    fourBytes |= (temp << 24);

    UCB1IFG &= ~UCRXIFG;
    UCB1IE |= UCRXIE;                           // Enable USCI0 RX interrupt

    return fourBytes;
}

void writeSpiByte(uint8_t byteData)
{
    UCB1TXBUF = byteData;                              // Transmit characters
    while(!(UCB1IFG & UCTXIFG));
    UCB1IFG &= ~UCTXIFG;
}

void writeSpiShort(uint16_t shortData)
{
    UCB1TXBUF = (shortData & 0xff);                              // Transmit characters
    while(!(UCB1IFG & UCTXIFG));
    UCB1IFG &= ~UCTXIFG;

    UCB1TXBUF = ((shortData>>8) & 0xff);                              // Transmit characters
    while(!(UCB1IFG & UCTXIFG));
    UCB1IFG &= ~UCTXIFG;
}

void writeSpiLong(uint32_t longData)
{
    UCB1TXBUF = (longData & 0xff);                              // Transmit characters
    while(!(UCB1IFG & UCTXIFG));
    UCB1IFG &= ~UCTXIFG;

    UCB1TXBUF = ((longData>>8) & 0xff);                              // Transmit characters
    while(!(UCB1IFG & UCTXIFG));
    UCB1IFG &= ~UCTXIFG;

    UCB1TXBUF = ((longData>>16) & 0xff);                              // Transmit characters
    while(!(UCB1IFG & UCTXIFG));
    UCB1IFG &= ~UCTXIFG;

    UCB1TXBUF = ((longData>>24) & 0xff);                              // Transmit characters
    while(!(UCB1IFG & UCTXIFG));
    UCB1IFG &= ~UCTXIFG;
}
