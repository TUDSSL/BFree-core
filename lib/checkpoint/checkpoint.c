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

/* .data section */
extern uint32_t _srelocate;
extern uint32_t _erelocate;

/* .bss section */
extern uint32_t _sbss;
extern uint32_t _ebss;

/* stack */
extern uint32_t _estack;


/* Register checkpoint
 * Registers are copied to `registers[]` in the SVC_Handler ISR
 * For the register layout in the array check `checkpoint_svc.s
 */
volatile uint32_t registers[17];
volatile uint32_t *registers_top = &registers[17]; // one after the end
volatile uint32_t checkpoint_svc_restore = 0;

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
  } while (0)

/*
 * Atomic flag manipulation
 */
#define CHECKPOINT_FLAG         (1<<0)
#define CHECKPOINT_FLAG_MASK    CHECKPOINT_FLAG

typedef uint32_t atomic_flag_t;

volatile atomic_flag_t checkpoint_flag = 0;
static inline atomic_flag_t cp_get_atomic_flag(void)
{
    return checkpoint_flag;
}

static inline void cp_set_atomic_flag(atomic_flag_t flag)
{
    checkpoint_flag = flag;
}

static inline atomic_flag_t checkpoint_other(atomic_flag_t flag)
{
    flag ^= CHECKPOINT_FLAG;
    return flag;
}

static inline void checkpoint_prepare(volatile atomic_flag_t *new_flag)
{
    *new_flag = cp_get_atomic_flag();
    *new_flag = checkpoint_other(*new_flag);
}

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
__attribute__((always_inline))
static inline void checkpoint_registers(void) {

    CP_SAVE_REGISTERS();

    if (checkpoint_svc_restore == 0) {
        // Send the registers
        printf("r%d\r\n", sizeof(registers));

        // Send the data
        write_serial_raw((char *)registers, sizeof(registers));
        mp_hal_stdout_tx_str("\r\n"); // newline after the checkpoint to keep it readable
    }
}

static void restore_registers(void) {
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

void checkpoint_memory(void) {
    uint32_t *sp = _get_sp();

    //int data_size = &_erelocate - &_srelocate;
    //int bss_size = &_ebss - &_sbss;
    int stack_size = &_estack - sp;

    //char *data_ptr = (char *)&_srelocate;
    //char *bss_ptr = (char *)&_sbss;
    char *stack_ptr = (char *)sp;

    //printf(".stack\t[%p-%p,%d]\r\n", sp, &_estack, stack_size);
    //printf(".data\t[%p-%p,%d]\r\n", &_srelocate, &_erelocate, data_size);
    //printf(".bss\t[%p-%p,%d]\r\n", &_sbss, &_ebss, bss_size);

    checkpoint_memory_region(stack_ptr, stack_size);
    //checkpoint_memory_region(data_ptr, data_size);
    //checkpoint_memory_region(bss_ptr, bss_size);
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


__attribute__((noinline))
static int pyrestore_process(void) {
    char *addr_start;
    char *addr_end;
    ssize_t size;

    // TEST: fill `random_data` with all 12
    memset((void *)random_data, 12, sizeof(random_data));

    //print_register_buffer();

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
                mp_hal_delay_ms(1000);
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

    //print_random_data();
    //print_register_buffer();

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



#if 0
static void checkpoint_exec(void) {
    // Send the unique key to signal the PC that we are about to send a checkpoint
    printf("%s", UNIQUE_CP_START_KEY);

    checkpoint_memory((char *)random_data, sizeof(random_data));
    checkpoint_registers();

    // Send the unique key to signal the PC that we are done sending checkpoint data
    printf("%s", UNIQUE_CP_END_KEY);
}
#endif

int checkpoint(void)
{
    /* Must be volatile as this function is also the restore point
     * and the flag is used to check if it is a restore or not
     */
    volatile atomic_flag_t new_flag;
    int restore_state;

    // Prepare for the next checkpoint
    checkpoint_prepare(&new_flag);

    // Send the unique key to signal the PC that we are about to send a checkpoint
    printf("%s", UNIQUE_CP_START_KEY);

    //checkpoint_memory_region((char *)random_data, sizeof(random_data));
    checkpoint_memory();
    checkpoint_registers();

    // NB: restore point
    if (cp_get_atomic_flag() == new_flag) {
        /* Restored */
        restore_state = 1;
        printf("%s", "Restored\r\n");
    } else {
        /* Normal operation */
        restore_state = 0;
        cp_set_atomic_flag(new_flag);

        // Send the unique key to signal the PC that we are done sending checkpoint data
        printf("%s", UNIQUE_CP_END_KEY);
    }

    return restore_state;
}
