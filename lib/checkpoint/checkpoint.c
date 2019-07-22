#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "py/mpconfig.h"

#include "supervisor/serial.h"
#include "py/mphal.h"
#include "supervisor/usb.h"
#include "tusb.h"

#include "lib/checkpoint/checkpoint.h"

#define UNIQUE_CP_START_KEY "##CHECKPOINT_START##\n"
#define UNIQUE_CP_END_KEY "##CHECKPOINT_END##\n"
#define UNIQUE_RESTORE_START_KEY "##CHECKPOINT_RESTORE##\n"

extern uint32_t _esafestack;
static volatile uint32_t *pyrestore_return_stack; // If we want to return to the stack (so we don't restore a register checkpoint)

void write_serial_raw(char *data, size_t length) {
    uint32_t count = 0;
    while (count < length && tud_cdc_connected()) {
        count += tud_cdc_write(data + count, length - count);
        usb_background();
    }
}

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
char random_data[12] = {1,2,3,4,5,6,7,8,9,10,11,12};
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

    // Signal a restore
    printf("%s", UNIQUE_RESTORE_START_KEY);

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
void pyrestore(void) {
    pyrestore_return_stack = _get_sp();
    _set_sp(&_esafestack); // Set SP to the safe stack

    //// WORKING ON THE SAFE STACK
    pyrestore_process();

    // Restore the old SP
    _set_sp((uint32_t *)pyrestore_return_stack);
}



/*
 * Checkpoint to PC
 *  M -> P: UNIQUE_CP_START_KEY     // signal the PC app that we wan't to send a checkpoint
 *  // SEGMENT CP
 *  M -> P: 's'                     // request to send a segment checkpoint (memory)
 *  M -> P: $addr_start:$addr_end   // send the address range
 *  M -> P: data[0:$size]           // send the checkpoint data
 *  // REGISTER CP
 *  M -> P: 'r'                     // request to send a register checkpoint
 *  M -> P: $registers[0:15]        // send the register values
 *  // END CP
 *  M -> P: UNIQUE_CP_END_KEY       // Continue
 */


/*
 * ARM m0 registers:
 *  r0, r1, r2, r3: function arguments
 *  r4, r5, r6, r7, r8: normal registers
 *  r9: normal register / real frame pointer
 *  r10: stack limit
 *  r11: argument pointer
 *  r12: scratch register
 *  r13: stack pointer
 *  r14: link register
 *  r15: program counter
 */
static volatile uint32_t registers[16];
void checkpoint_registers(void) {
    // For some reason placing it all in the same __asm__ causes a compiler error
    __asm__ volatile (
            "MOV %[r0], r0 \n\t"
            "MOV %[r1], r1 \n\t"
            "MOV %[r2], r2 \n\t"
            "MOV %[r3], r3 \n\t"
            "MOV %[r4], r4 \n\t"
            "MOV %[r5], r5 \n\t"
            "MOV %[r6], r6 \n\t"
            "MOV %[r7], r7 \n\t"
            "MOV %[r8], r8 \n\t"
            :   [r0] "=r" (registers[0]),
                [r1] "=r" (registers[1]),
                [r2] "=r" (registers[2]),
                [r3] "=r" (registers[3]),
                [r4] "=r" (registers[4]),
                [r5] "=r" (registers[5]),
                [r6] "=r" (registers[6]),
                [r7] "=r" (registers[7]),
                [r8] "=r" (registers[8])
            : /* input */
            : "memory" /* clobber */
    );
    __asm__ volatile (
            "MOV %[r9], r9 \n\t"
            "MOV %[r10], r10 \n\t"
            "MOV %[r11], r11 \n\t"
            "MOV %[r12], r12 \n\t"
            "MOV %[r13], r13 \n\t"
            "MOV %[r14], r14 \n\t"
            "MOV %[r15], r15 \n\t"
            :   [r9] "=r" (registers[9]),
                [r10] "=r" (registers[10]),
                [r11] "=r" (registers[11]),
                [r12] "=r" (registers[12]),
                [r13] "=r" (registers[13]),
                [r14] "=r" (registers[14]),
                [r15] "=r" (registers[15])
            : /* input */
            : "memory" /* clobber */
    );

    // Send the registers
    mp_hal_stdout_tx_str("r");

    // Send the data
    write_serial_raw((char *)registers, sizeof(registers));
    mp_hal_stdout_tx_str("\r\n"); // newline after the checkpoint to keep it readable
}

static void checkpoint_memory(char *start, size_t size) {
    char *end;

    if (size == 0) {
        // error
        return;
    }

    end = &start[size];

    mp_hal_stdout_tx_str("s");
    printf("%lx:%lx\r\n", (uint32_t)start, (uint32_t)end);

    // Send the data
    write_serial_raw(start, size);
    mp_hal_stdout_tx_str("\r\n"); // newline after the checkpoint to keep it readable
}

void checkpoint(void) {
    // Send the unique key to signal the PC that we are about to send a checkpoint
    printf("%s", UNIQUE_CP_START_KEY);

    checkpoint_memory((char *)random_data, sizeof(random_data));
    checkpoint_registers();

    // Send the unique key to signal the PC that we are done sending checkpoint data
    printf("%s", UNIQUE_CP_END_KEY);
}
