#ifndef MPY_COMM_H__
#define MPY_COMM_H__

#include <stdlib.h>
#include <msp430.h>

void mpy_comm_init(void);
int mpy_write(char *src, size_t size);
int mpy_read(char *dst, size_t size);

/* Fast writing */
int mpy_write_dma(char *src, size_t size);
int mpy_write_dma_blocking(char *src, size_t size);

int mpy_read_dma(char *dst, size_t size);
int mpy_read_dma_blocking(char *dst, size_t size);

#define WR_PIN BIT2

static inline void mpy_wr_high(void)
{
    P1OUT |= WR_PIN;
}

static inline void mpy_wr_low(void)
{
    P1OUT &= ~WR_PIN;
}

static inline char mpy_write_byte(char data)
{
    //printf("Write byte send data: 0x%x\r\n", data);
    UCB1TXBUF = data;
    mpy_wr_high();
    while(!(UCB1IFG & UCTXIFG));
    UCB1IFG &= ~UCTXIFG;
    mpy_wr_low();

    // Also clear the read IF (discard)
    UCB1IFG &= ~UCRXIFG;
    while(!(UCB1IFG & UCRXIFG));
    data = UCB1RXBUF;
    UCB1IFG &= ~UCRXIFG;

    UCB1TXBUF = 0;

    //printf("Write byte return data: 0x%x\r\n", data);

    return data;
}

static inline char mpy_read_byte(void)
{
    mpy_wr_high();
    while(!(UCB1IFG & UCRXIFG));
    mpy_wr_low();
    char data = UCB1RXBUF;
    UCB1IFG &= ~UCRXIFG;

    // Alsoc clear the write IF (discard)
    UCB1IFG &= ~UCTXIFG;

    //printf("Read byte return data: 0x%x\r\n", data);

    return data;
}

#endif /* MPY_COMM_H__ */
