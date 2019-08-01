#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "py/mpconfig.h"

#include "supervisor/serial.h"
#include "py/mphal.h"
#include "supervisor/usb.h"
#include "tusb.h"

#include "lib/checkpoint/checkpoint.h"

#define CP_REGISTERS    (1)
#define CP_STACK        (1)
#define CP_DATA         (1)
#define CP_BSS          (1)
#define CP_GARBAGE      (0)

#define UNIQUE_CP_START_KEY "##CHECKPOINT_START##\n"
#define UNIQUE_CP_END_KEY "##CHECKPOINT_END##\n"
#define UNIQUE_RESTORE_START_KEY "##CHECKPOINT_RESTORE##\n"

/* .data section */
extern uint32_t _srelocate;
extern uint32_t _erelocate_cp;

/* .bss section */
extern uint32_t _sbss;
extern uint32_t _ebss_cp;

/* stack */
extern uint32_t _estack;

/* Safe stack (not checkpointed)
 * restoring a checkpoint happens on this stack
 */
extern uint32_t _esafestack;


/* Register checkpoint
 * Registers are copied to `registers[]` in the SVC_Handler ISR
 * For the register layout in the array check `checkpoint_svc.s
 */
volatile uint32_t registers[17];
const volatile uint32_t *registers_top = &registers[17]; // one after the end
volatile uint32_t checkpoint_svc_restore = 0;

static inline uint32_t checkpoint_restored(void) {
    return (uint32_t)checkpoint_svc_restore;
}

#define CP_SAVE_REGISTERS()                                                    \
  do {                                                                         \
    checkpoint_svc_restore = 0;                                                \
    __asm__ volatile("SVC 42");                                                \
    __ISB();                                                                   \
  } while (0)

#define CP_RESTORE_REGISTERS()                                                 \
  do {                                                                         \
    checkpoint_svc_restore = 1;                                                \
    __asm__ volatile("SVC 43");                                                \
    __ISB();                                                                   \
  } while (0)

/*
 * Atomic flag manipulation
 */
#define CHECKPOINT_FLAG         (1<<0)
#define CHECKPOINT_FLAG_MASK    CHECKPOINT_FLAG

void write_serial_raw(char *data, size_t length) {
    size_t count = 0;
    while (count < length && tud_cdc_connected()) {
        count += tud_cdc_write(data + count, length - count);
        usb_background();
    }
}

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

__attribute__((always_inline))
static inline void checkpoint_registers(void) {

    CP_SAVE_REGISTERS();

    if (checkpoint_restored() == 0) {
        // Send the registers
        printf("r%d\r\n", sizeof(registers));

        // Send the data
        write_serial_raw((char *)registers, sizeof(registers));
        mp_hal_stdout_tx_str("\r\n"); // newline after the checkpoint to keep it readable
    }
}

static inline void restore_registers(void) {
    CP_RESTORE_REGISTERS();
}

static void checkpoint_memory_region(char *start, size_t size) {
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
 volatile size_t stack_size;
void checkpoint_memory(void) {

#if CP_STACK
    uint32_t *sp = _get_sp();
    stack_size = (size_t)&_estack - (size_t)sp;
    char *stack_ptr = (char *)sp;
    checkpoint_memory_region(stack_ptr, stack_size);
#endif

#if CP_DATA
    size_t data_size = (size_t)&_erelocate_cp - (size_t)&_srelocate;
    char *data_ptr = (char *)&_srelocate;
    checkpoint_memory_region(data_ptr, data_size);
#endif

#if CP_BSS
    size_t bss_size = (size_t)&_ebss_cp - (size_t)&_sbss;
    char *bss_ptr = (char *)&_sbss;
    checkpoint_memory_region(bss_ptr, bss_size);
#endif

    //printf(".stack\t[%p-%p,%d]\r\n", sp, &_estack, stack_size);
    //printf(".data\t[%p-%p,%d]\r\n", &_srelocate, &_erelocate, data_size);
    //printf(".bss\t[%p-%p,%d]\r\n", &_sbss, &_ebss, bss_size);
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

// Ideally there would be some kind of timeout
static ssize_t writeback_register_stream(void) {
    size_t cnt;
    char *c_registers = (char *)&registers;

    for (cnt=0; cnt<(size_t)sizeof(registers); cnt++) {
        char d = mp_hal_stdin_rx_chr();
        c_registers[cnt] = d;
    }

    return cnt;
}

// For debugging only
char random_data[12] = {1,2,3,4,5,6,7,8,9,10,11,12};
void print_random_data(void) {
    mp_hal_stdout_tx_str("random_data: ");
    for (unsigned int i=0; i<sizeof(random_data); i++) {
        printf("%d, ", (int)random_data[i]);
    }
    mp_hal_stdout_tx_str("\r\n");
}

void print_register_buffer(void) {
    printf("Register buffer:\r\n");
    for (unsigned int i=0; i<16; i++) {
        printf("r%d: %lx\r\n", i, registers[i]);
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
__attribute__((noinline))
static int pyrestore_process(void) {
    char *addr_start;
    char *addr_end;
    ssize_t size;

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
                mp_hal_stdout_tx_str("a"); // send ACK
                size = writeback_register_stream();
                printf("%d\r\n", size); // send size ACK
                restore_registers();
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

    return 1;
}

/*
 * Because we restore the stack we can't operate on the normal stack
 * So we need to switch the stack to a dedicated one (_esafestack) and restore
 * the stack afterwards
 *
 * NB. This function can NOT use the stack or funny things will happen
 */
__attribute__((noinline))
void pyrestore(void) {
    /* If we want to return to the stack (so we don't restore a register checkpoint) */
    static volatile uint32_t *pyrestore_return_stack;

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

int checkpoint(void)
{
    int restore_state;

    // Send the unique key to signal the PC that we are about to send a checkpoint
    printf("%s", UNIQUE_CP_START_KEY);

    checkpoint_memory();

#if CP_REGISTERS
    checkpoint_registers();
#endif

    // NB: restore point
    restore_state = checkpoint_restored();
    if (restore_state) {
        /* Restored */
        printf("%s", "Restored\r\n");
    } else {
        /* Normal operation */
        // Send the unique key to signal the PC that we are done sending checkpoint data
        printf("%s", UNIQUE_CP_END_KEY);
    }

    return restore_state;
}
