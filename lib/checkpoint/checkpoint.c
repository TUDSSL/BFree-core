#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "supervisor/serial.h"
#include "py/mphal.h"

#include "lib/checkpoint/checkpoint.h"

extern uint32_t _esafestack;
static volatile uint32_t *pyrestore_return_stack; // If we want to return to the stack (so we don't restore a register checkpoint)

/*
 * Restore process states
 *  P = python restore script, M = metro board
 *
 *(1)   if input byte == 'c': DONE
 *(2)   if input byte == 's': restore memory segment
 *(3)   if input byte == 'r': restore register
 *
 *(4)   s:
 *(5)       send 'a'                        // Ack
 *(6)       get $addr_start:$addr_end       // in hex
 *(7)       send $size                      // in dec $addr_end-$addr_start as ACK
 *(8)       get data[0:$size]               // receive all the data restore bytes
 *(9)       send $size                      // in dec as ack
 *(10)      GOTO (1)
 *
 */


/*
 * Because we restore the stack we can't operate on the normal stack
 * So we need to switch the stack to a dedicated one (_esafestack) and restore
 * the stack afterwords
 */

__attribute__((always_inline))
static inline uint32_t * _get_sp(void) {
    uint32_t *sp;

    __asm__ volatile (
            "MOV %[stack_ptr], SP \n\t"
            :   [stack_ptr] "=r" (sp)/* output */
            : /* input */
            : "memory" /* clobber */
    );

    return sp;
}

__attribute__((always_inline))
static inline void _set_sp(uint32_t *sp) {
    __asm__ volatile (
            "MOV SP, %[stack_ptr] \n\t"
            : /* output */
            :   [stack_ptr] "r" (sp)/* input */
            : "memory" /* clobber */
    );
}


static ssize_t process_segment_addr_range(char **addr_start, char **addr_end) {
    char command[8+1+8+1]; // Enough for two 32-bit hex string numbers (NB no 0x prefix)
    int idx = 0;

    // Expect $addr_start:$addr_end as a hex string
    while (1) {
        int c = mp_hal_stdin_rx_chr();

        if (c == '\n') {
            command[idx] = '\0';
            break;
        } else {
            // No error detection, everything else is assumed to be a hex addr
            command[idx++] = (char)c;
        }
    }

    // Decode the hex addr format
    char *endptr;
    *addr_start = (char *)strtol(command, &endptr, 16);
    if (*endptr != ':') {
        // Error parsing command
        return 0;
    }
    char *cmd_ptr = &endptr[1]; // skip ':'
    *addr_end = (char *)strtol(cmd_ptr, &endptr, 16);

    if (*addr_start > *addr_end) {
        // start addr > endaddr
        return -1;
    }

    ssize_t size = *addr_end - *addr_start;

    return size;
}

// Ideally there would be some kind of timeout
static ssize_t writeback_memory_stream(char *addr_start, ssize_t size) {
    size_t cnt;

    // TODO error handling
    if (size < 1)
        return size;

    for (cnt=0; cnt<(size_t)size; cnt++) {
        char d = mp_hal_stdin_rx_chr();
        addr_start[cnt] = d;
    }

    return cnt;
}

// For debugging only
char random_data[12];
static void print_random_data(void) {
    mp_hal_stdout_tx_str("random_data: ");
    for (unsigned int i=0; i<sizeof(random_data); i++) {
        printf("%d, ", (int)random_data[i]);
    }
    mp_hal_stdout_tx_str("\r\n");
}


__attribute__((noinline))
static int pyrestore_process(void) {
    char *addr_start;
    char *addr_end;
    ssize_t size;

    // TEST: fill `random_data` with all 12
    memset((void *)random_data, 12, sizeof(random_data));

    while (1) {
        int c = mp_hal_stdin_rx_chr();
        int done = 0;

        switch (c) {
            case 'c':
                done = 1;
                break;
            case 'r':
                break;
            case 's':
                mp_hal_stdout_tx_str("a"); // send ACK
                size = process_segment_addr_range(&addr_start, &addr_end);
                printf("%d\r\n", (int)size); // send size ACK
                size = writeback_memory_stream(addr_start, size);
                printf("%d\r\n", (int)size); // send size ACK
                break;
            default:
                // Garbage or unknown command
                break;
        }

        if (done)
            break;
    }

    print_random_data();

    //mp_hal_delay_ms(1000);

    return 1;
}

// NB. This function can NOT use the stack or funny things will happen
__attribute__((noinline))
void pyrestore(void)
{
    pyrestore_return_stack = _get_sp();
    _set_sp(&_esafestack); // Set SP to the safe stack

    //// WORKING ON THE SAFE STACK
    pyrestore_process();

    // Restore the old SP
    _set_sp((uint32_t *)pyrestore_return_stack);
}

