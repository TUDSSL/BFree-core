#include <msp430.h>
#include "eusci_b_spi.h"

#include "mpy_comm.h"

// SPI port 5
#define SPI_MOSI BIT0   // P5.0
#define SPI_MISO BIT1   // P5.1
#define SPI_SCLK BIT2   // P5.2


#define CHECK_SIZE(size_) do {  \
    if (size_ == 0) return 1;   \
} while (0)

void mpy_comm_init(void)
{
    /* Select SPI functionality for the pins */
    P5SEL1 &= ~(SPI_MOSI | SPI_MISO | SPI_SCLK);
    P5SEL0 |= (SPI_MOSI | SPI_MISO | SPI_SCLK);

    /* Initialize the WR pin to signal when the MSP is done processing */
    P1OUT &= ~WR_PIN;
    P1DIR |= WR_PIN;

#ifdef SPI_MASTER_TEST // SPI Master for testing
    /* Configure SPI */
    EUSCI_B_SPI_initMasterParam param;
    param.selectClockSource = EUSCI_B_SPI_CLOCKSOURCE_SMCLK;
    param.clockSourceFrequency = 16000000; // 16MHz
    param.desiredSpiClock = 100000; // 8MHz
    param.msbFirst = EUSCI_B_SPI_LSB_FIRST;
    param.clockPhase = EUSCI_B_SPI_CLOCKPOLARITY_INACTIVITY_LOW;
    param.clockPolarity = EUSCI_B_SPI_3PIN;

    EUSCI_B_SPI_initMaster(EUSCI_B1_BASE, &param);
    EUSCI_B_SPI_enable(EUSCI_B1_BASE);
    EUSCI_B_SPI_clearInterrupt(EUSCI_B0_BASE,
            EUSCI_B_SPI_RECEIVE_INTERRUPT);
#else // SPI Slave
    /* Configure SPI */
    EUSCI_B_SPI_initSlaveParam param;
    param.msbFirst = EUSCI_B_SPI_MSB_FIRST;
    param.clockPhase = EUSCI_B_SPI_PHASE_DATA_CAPTURED_ONFIRST_CHANGED_ON_NEXT;
    param.clockPolarity = EUSCI_B_SPI_CLOCKPOLARITY_INACTIVITY_LOW;
    param.spiMode = EUSCI_B_SPI_3PIN;

    EUSCI_B_SPI_initSlave(EUSCI_B1_BASE, &param);
    EUSCI_B_SPI_enable(EUSCI_B1_BASE);
    EUSCI_B_SPI_clearInterrupt(EUSCI_B0_BASE,
            EUSCI_B_SPI_RECEIVE_INTERRUPT);
#endif
}


int mpy_write(char *src, size_t size)
{
    CHECK_SIZE(size);

    for (size_t i=0; i<size; i++) {
        mpy_write_byte(src[i]);
    }

    return 0;
}

int mpy_read(char *dst, size_t size)
{
    CHECK_SIZE(size);

    for (size_t i=0; i<size; i++) {
        dst[i] = mpy_read_byte();
    }

    return 0;
}

int mpy_write_dma(char *src, size_t size)
{
    DMACTL1 |= DMA3TSEL__UCB1TXIFG;  // UCB1TXIFG if DMA Channel 3

    __data16_write_addr((intptr_t) &DMA3SA,(intptr_t) src);
    __data16_write_addr((intptr_t) &DMA3DA,(intptr_t) &UCB1TXBUF);

    DMA3SZ = size;
    DMA3CTL = DMADT_0 | DMASRCINCR_3 | DMADSTBYTE__BYTE | DMASRCBYTE__BYTE | DMAEN;

    mpy_wr_high();

    // Trigger transfer
    UCB1IFG &= ~UCTXIFG;
    UCB1IFG |=  UCTXIFG;

    return 0;
}

int mpy_write_dma_blocking(char *src, size_t size)
{
    int ret = mpy_write_dma(src, size);
    while (!(DMA3CTL & DMAIFG)); // Wait for DMA to finish
    mpy_wr_low();

    return ret;
}

int mpy_read_dma(char *dst, size_t size)
{
    DMACTL1 |= DMA3TSEL__UCB1RXIFG;  // UCB1RXIFG if DMA Channel 3

    __data16_write_addr((intptr_t) &DMA3SA,(intptr_t) &UCB1RXBUF);
    __data16_write_addr((intptr_t) &DMA3DA,(intptr_t) dst);

    DMA3SZ = size;
    DMA3CTL = DMADT_0 | DMADSTINCR_3 | DMADSTBYTE__BYTE | DMASRCBYTE__WORD | DMAEN;

    mpy_wr_high();

    // Trigger transfer
    //UCB1IFG &= ~UCTXIFG;
    //UCB1IFG |=  UCTXIFG;

    return 0;
}

int mpy_read_dma_blocking(char *dst, size_t size)
{
    int ret = mpy_read_dma(dst, size);
    while (!(DMA3CTL & DMAIFG)); // Wait for DMA to finish
    mpy_wr_low();

    return ret;
}
