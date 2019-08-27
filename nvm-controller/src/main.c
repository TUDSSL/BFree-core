#include <stdio.h>
#include <string.h>
//#include "debug.h"
#include "ports.h"
#include "mpy_comm.h"

#include "checkpoint_ctrl.h"



#if 1
volatile char read_buffer[1024];
int main(void)
{
    WDTCTL = WDTPW | WDTHOLD; // Stop WDT
    portConfig();
    clockConfig();
    ledConfig();

    /* SPI communication with Metro */
    mpy_comm_init();

    PM5CTL0 &= ~LOCKLPM5;
    __enable_interrupt();

    // Init checkpoint
    checkpoint_update();

    printf("Waiting for sync\r\n");
    process_reboot_sync();
    mpy_wr_high();
    mpy_wr_low();
    mpy_wr_high();
    mpy_wr_low();

    printf("Start communication\r\n");
    mpy_comm_start();
    while (1) {
        process_dispatch();
    }
}
#else

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD; // Stop WDT
    portConfig();
    clockConfig();
    ledConfig();

    P5OUT &= ~BIT3;
    P5DIR |= BIT3;

    /* SPI communication with Metro */
    mpy_comm_init();

    PM5CTL0 &= ~LOCKLPM5;
    __enable_interrupt();

    checkpoint_update();
    segment_t *sega_0 = checkpoint_segment_alloc(10);
    segment_t *sega_1 = checkpoint_segment_alloc(10);

    memset(sega_0->data, 42, 10);
    memset(sega_1->data, 62, 10);

    checkpoint_commit();
    segment_t *segb_0 = checkpoint_segment_alloc(10);
    segment_t *segb_1 = checkpoint_segment_alloc(10);

    memset(segb_0->data, 80, 10);
    memset(segb_1->data, 99, 10);

    checkpoint_commit();

    segment_iter_t si;
    checkpoint_get_segment_iter(&si);
    while (si.segment != NULL) {
        printf("Segment addr: %p\r\n", si.segment);
        checkpoint_get_next_segment(&si);
    }



    while (1) {
        for (int i=0; i<1000; i++) {
            P5OUT ^= BIT3;
        }
    }
}
#endif


#if 0
/*
 * SPI ISR
 */
volatile uint8_t rxBuf = 0;
void __attribute__ ((interrupt(EUSCI_B1_VECTOR))) USCI_B1_ISR (void)
{
    switch(__even_in_range(UCB1IV, USCI_SPI_UCTXIFG))
    {
        case USCI_NONE:
            break;
        case USCI_SPI_UCRXIFG:
            rxBuf = UCB1RXBUF;
            UCB1IFG &= ~UCRXIFG;
            break;
        case USCI_SPI_UCTXIFG:
            break;
        default:
            break;
    }
}
#endif
