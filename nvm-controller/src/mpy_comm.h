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

static inline void mpy_write_byte(char data)
{
    UCB1TXBUF = data;
    while(!(UCB1IFG & UCTXIFG));
    UCB1IFG &= ~UCTXIFG;
}

static inline char mpy_read_byte(void)
{
    while(!(UCB1IFG & UCRXIFG));
    char data = UCB1RXBUF;
    UCB1IFG &= ~UCRXIFG;

    return data;
}

#endif /* MPY_COMM_H__ */
