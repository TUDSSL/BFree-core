#include <stdio.h>
#include <string.h>
//#include "debug.h"
#include "ports.h"
#include "mpy_comm.h"

#include "checkpoint_ctrl.h"

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

    //printf("Waiting for sync\r\n");
    //process_reboot_sync();

    printf("Start communication\r\n");
    mpy_comm_start();
    while (1) {
        process_dispatch();
    }
}
