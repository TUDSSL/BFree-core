#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "py/mpconfig.h"

#include "supervisor/serial.h"
#include "py/mphal.h"
#include "supervisor/usb.h"
#include "tusb.h"

#include "supervisor/memory.h"

// Intermittent group additions TODO: remove duplicates
/**
 * Pin mappings.
 * Supports sending and recieving bytes to/from the external Non-volatile memory controller,
 * which is a MSP430FR59941 in the current version.
 *
 * PIN ASSIGNMENTS  for the Interface to the MSP430.
 * ===============================================
 * CP  |     Function    |  MSP |  M0
 * ===============================================
 * D13 | CLK             | P5.2 | PA17/SERCOM1+3.1
 * D12 | SOMI            | P5.1 | PA19/SERCOM1+3.3
 * D11 | SIMO            | P5.0 | PA16/SERCOM1+3.0
 * D10 | CS              | P5.3 | PA18/SERCOM1+3.2
 * D9  | NV_WRITE / GPIO1| P1.7 | PA07
 * D8  | VCAP_HALF / ADC | P7.4 | PA06
 * D7  | NV_READ         | P1.6 | PA21
 * D6  | PFAIL           | P2.0 | PA20
 *
 *
 */
#include "hal/include/hal_gpio.h"
#include "atmel_start_pins.h"
#include "instance/sercom1.h"
#include "shared-bindings/busio/SPI.h"
#include "shared-bindings/digitalio/DigitalInOut.h"
#include "shared-bindings/microcontroller/Pin.h"
#include "supervisor/shared/external_flash/common_commands.h"
#include "supervisor/shared/external_flash/external_flash.h"
#include "supervisor/filesystem.h"
#include "py/mpconfig.h"

#include "lib/checkpoint/checkpoint.h"

#define CP_REGISTERS    (1)
#define CP_STACK        (0)
#define CP_DATA         (1)
#define CP_BSS          (1)
#define CP_ALLOCATIONS  (1)

#define GDB_LOG_CP      (1)


/* Debug options */
#define CP_CHECKPOINT_DISABLE   (0) // Disable checkpoints
#define CP_RESTORE_DISABLE      (0) // Disable restores
#define CP_IGNORE_PENDING       (0) // Checkpoint irregardless of pending status
#define CP_PRINT_PENDING        (0) // Print pending timing info
#define CP_PRINT_COMMIT         (0) // Print checkpoint commit timing info


/*
 * Used by GDB as watchpoints
 */
#if GDB_LOG_CP
volatile int _gdb_break_me_cnt = 0;
__attribute__((noinline))
void gdb_break_me(void) {++_gdb_break_me_cnt;}
#endif

/*
 * One byte commands
 */
#define CPCMND_REQUEST_CHECKPOINT   'x'
#define CPCMND_REQUEST_RESTORE      'y'
#define CPCMND_SEGMENT              's'
#define CPCMND_REGISTERS            'r'
#define CPCMND_ACK                  'a'
#define CPCMND_CONTINUE             'c'
#define CPCMND_DEL                  'd'

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


/* Checkpoint pending flag */
volatile int checkpoint_pending = 0;

/* Register checkpoint
 * Registers are copied to `registers[]` in the SVC_Handler ISR
 * For the register layout in the array check `checkpoint_svc.s
 */
volatile uint32_t registers[17];
const volatile uint32_t *registers_top = &registers[17]; // one after the end
volatile uint32_t checkpoint_svc_restore = 0;

#define checkpoint_restored() checkpoint_svc_restore

#define CP_SAVE_REGISTERS()                                                    \
  do {                                                                         \
    checkpoint_svc_restore = 0;                                                \
    __asm__ volatile("SVC 42");                                                \
    __DSB();                                                                   \
    /*__ISB();*/                                                                   \
  } while (0)

#define CP_RESTORE_REGISTERS()                                                 \
  do {                                                                         \
    checkpoint_svc_restore = 1;                                                \
    __asm__ volatile("SVC 43");                                                \
  } while (0)

/*
 * NVM communication
 */
struct spi_m_sync_descriptor SPI_M_SERCOM1;
busio_spi_obj_t nv_spi_bus;
digitalio_digitalinout_obj_t cs_pin_nv;

digitalio_digitalinout_obj_t wr_pin_nv;
digitalio_digitalinout_obj_t rst_pin_nv;

void nvm_wait_process(void) {
    // Wait untill the NVM signals it's ready
    while (common_hal_digitalio_digitalinout_get_value(&wr_pin_nv) == false) {
        filesystem_background();
        usb_background();
    }
}

void nvm_comm_init(void) {
     // Init the CS pin for NV memory controller
    cs_pin_nv.base.type = &digitalio_digitalinout_type;
    common_hal_digitalio_digitalinout_construct(&cs_pin_nv, &pin_PA18);

    // NV Set CS high (disabled).
    common_hal_digitalio_digitalinout_switch_to_output(&cs_pin_nv, true, DRIVE_MODE_PUSH_PULL);
    common_hal_digitalio_digitalinout_never_reset(&cs_pin_nv);
    common_hal_digitalio_digitalinout_set_value(&cs_pin_nv, true);

    // SPI for NV comms
    nv_spi_bus.base.type = &busio_spi_type;
    nv_spi_bus.spi_desc = SPI_M_SERCOM1;
    common_hal_busio_spi_construct(&nv_spi_bus, &pin_PA17, &pin_PA16, &pin_PA19);
    common_hal_busio_spi_configure(&nv_spi_bus, 4000000, 0, 0, 8);
    common_hal_busio_spi_never_reset(&nv_spi_bus);

    // Init the WR pin for NV memory controller
    wr_pin_nv.base.type = &digitalio_digitalinout_type;
    common_hal_digitalio_digitalinout_construct(&wr_pin_nv, &pin_PA07);
    common_hal_digitalio_digitalinout_switch_to_input(&wr_pin_nv, PULL_DOWN);
    common_hal_digitalio_digitalinout_never_reset(&wr_pin_nv);

    // Init the reset pin for the NV memory controller
    rst_pin_nv.base.type = &digitalio_digitalinout_type;
    common_hal_digitalio_digitalinout_construct(&rst_pin_nv, &pin_PA21);
    common_hal_digitalio_digitalinout_switch_to_output(&rst_pin_nv, true, DRIVE_MODE_OPEN_DRAIN);
    common_hal_digitalio_digitalinout_never_reset(&rst_pin_nv);
    common_hal_digitalio_digitalinout_set_value(&rst_pin_nv, true);
}

void nvm_reset(void) {
    common_hal_busio_spi_unlock(&nv_spi_bus);
    common_hal_digitalio_digitalinout_set_value(&rst_pin_nv, false);
    for (int i=0; i<10000; i++) {
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
    }
    common_hal_digitalio_digitalinout_set_value(&rst_pin_nv, true);
}

void nvm_write(char *src, size_t len) {
    while (!common_hal_busio_spi_try_lock(&nv_spi_bus));
    common_hal_digitalio_digitalinout_set_value(&cs_pin_nv, false);

    nvm_wait_process();
    common_hal_busio_spi_write(&nv_spi_bus, (uint8_t *)src, len);

    common_hal_digitalio_digitalinout_set_value(&cs_pin_nv, true);
    common_hal_busio_spi_unlock(&nv_spi_bus);
}

void nvm_write_byte(char src_byte) {
    nvm_write(&src_byte, 1);
}

void nvm_write_per_byte(char *src, size_t len)
{
    for (size_t i=0; i<len; i++) {
        nvm_write_byte(src[i]);
    }
}

void nvm_read(char *dst, size_t len) {
    while (!common_hal_busio_spi_try_lock(&nv_spi_bus)) {}
    common_hal_digitalio_digitalinout_set_value(&cs_pin_nv, false);

    nvm_wait_process();
    common_hal_busio_spi_read(&nv_spi_bus, (uint8_t *)dst, len, 0);

    common_hal_digitalio_digitalinout_set_value(&cs_pin_nv, true);
    common_hal_busio_spi_unlock(&nv_spi_bus);
}

char nvm_read_byte(void) {
    char dst_byte;
    nvm_read(&dst_byte, 1);

    return dst_byte;
}

void nvm_read_per_byte(char *dst, size_t len) {
    for (size_t i=0; i<len; i++) {
        dst[i] = nvm_read_byte();
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
        char resp;

        // Send the registers
        nvm_write_byte(CPCMND_REGISTERS);

        // Send the register size
        registers_size_t register_size = sizeof(registers);
        nvm_write_per_byte((char *)&register_size, sizeof(registers_size_t));

        // Read ACK
        resp = nvm_read_byte();
        if (resp != CPCMND_ACK) {
            return;
        }

        // Send the registers
        nvm_write((char *)registers, register_size);

        // Read ACK
        resp = nvm_read_byte();
        if (resp != CPCMND_ACK) {
            while(1);
            return;
        }
    }
}

static inline void restore_registers(void) {
    CP_RESTORE_REGISTERS();
}

static void checkpoint_memory_region(char *start, size_t size) {
    char resp;
    char *end;
    segment_size_t addr_start, addr_end, segment_size;

    if (size == 0) {
        // error
        return;
    }

    end = &start[size];

    addr_start = (segment_size_t)start;
    addr_end = (segment_size_t)end;

    //printf("%lx:%lx\r\n", addr_start, addr_end);

    nvm_write_byte(CPCMND_SEGMENT);

    resp = nvm_read_byte();
    if (resp != CPCMND_ACK) {
        while(1);
        return;
    }

    nvm_write_per_byte((char *)&addr_start, sizeof(segment_size_t));
    nvm_write_per_byte((char *)&addr_end, sizeof(segment_size_t));

    // Read the size as ACK
    nvm_read_per_byte((char *)&segment_size, sizeof(segment_size_t));

    if (segment_size != size) {
        printf("Sizes do not match, ACK size: %ld, expected: %ld\r\n",
                segment_size, (segment_size_t)size);
        return;
    }

    // Send the data
    nvm_write(start, size);

    // ACK
    resp = nvm_read_byte();
    if (resp != CPCMND_ACK) {
        while(1);
        return;
    }
}

void checkpoint_memory(void) {

#if CP_STACK
    uint32_t *sp = _get_sp();
    size_t stack_size = (size_t)&_estack - (size_t)sp;
    char *stack_ptr = (char *)sp;
    checkpoint_memory_region(stack_ptr, stack_size);
    //printf(".stack\t[%p-%p,%d]\r\n", sp, &_estack, stack_size);
#endif

#if CP_DATA
    size_t data_size = (size_t)&_erelocate_cp - (size_t)&_srelocate;
    char *data_ptr = (char *)&_srelocate;
    checkpoint_memory_region(data_ptr, data_size);
    //printf(".data\t[%p-%p,%d]\r\n", &_srelocate, &_erelocate_cp, data_size);
#endif

#if CP_BSS
    size_t bss_size = (size_t)&_ebss_cp - (size_t)&_sbss;
    char *bss_ptr = (char *)&_sbss;
    checkpoint_memory_region(bss_ptr, bss_size);
    //printf(".bss\t[%p-%p,%d]\r\n", &_sbss, &_ebss_cp, bss_size);
#endif

#if CP_ALLOCATIONS
    supervisor_allocation *allocations;
    uint32_t *at_ptr;
    uint32_t table_size = supervisor_get_allocations(&allocations);
    //printf("Allocation table start address (%d): %p\r\n", (int)table_size, allocations);
    for (uint32_t i=0; i<table_size; i++) {
        at_ptr = allocations[i].ptr;
        if (at_ptr != NULL) {
            //printf("Checkpoint allocation index: %d, ptr: 0x%p, size: %d\r\n", (int)i, at_ptr, (int)allocations[i].length);
            checkpoint_memory_region((char *)at_ptr, allocations[i].length);
        }
    }
#endif

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
extern void common_hal_busio_i2c_restore(void);
extern void common_hal_digitalio_digitalinout_restore(void);
extern void common_hal_analogio_analogin_restore(void);

__attribute__((unused))
__attribute__((noinline))
static int pyrestore_process(void) {
    segment_size_t addr_start, addr_end, size;

    nvm_write_byte(CPCMND_REQUEST_RESTORE);

    bool restore_registers_pending = false;
    while (1) {
        int c = nvm_read_byte();
        int done = 0;

        switch (c) {
            case CPCMND_CONTINUE:
                gpio_set_pin_direction(PA09, GPIO_DIRECTION_OUT);
                gpio_set_pin_level(PA09, true);
                if (restore_registers_pending) {
//#if GDB_LOG_CP
//                    checkpoint_svc_restore = 1;
//                    gdb_break_me();
//#endif
                    common_hal_busio_i2c_restore();
                    common_hal_analogio_analogin_restore();
                    common_hal_digitalio_digitalinout_restore();
                    restore_registers();
                }

                done = 1;
                break;
            case CPCMND_REGISTERS:
                nvm_write_byte(CPCMND_ACK); // send ACK

                nvm_read((char *)&registers, sizeof(registers));

                nvm_write_byte(CPCMND_ACK); // send ACK

                //restore_registers();
                restore_registers_pending = true;
                break;
            case CPCMND_SEGMENT:
                nvm_write_byte(CPCMND_ACK); // send ACK

                nvm_read_per_byte((char *)&addr_start, sizeof(segment_size_t));
                nvm_read_per_byte((char *)&addr_end, sizeof(segment_size_t));

                size = addr_end - addr_start;
                nvm_write_per_byte((char *)&size, sizeof(segment_size_t)); // send size as ACK

                printf("Restoring segment: start=%p, end=%p, size=%d\r\n", (char *)addr_start, (char *)addr_end, (int)size);
                // Now the memory segment writeback starts
                nvm_read((char *)addr_start, size);

                nvm_write_byte(CPCMND_ACK); // send ACK

                break;
            default:
                /* Garbage or unknown command */
                // Try again to ask for a restore
                nvm_reset();
                nvm_write_byte(CPCMND_REQUEST_RESTORE);
                break;
        }

        if (done)
            break;
    }

    return 1;
}

void checkpoint_init(void)
{
    nvm_comm_init();
    nvm_reset();
    NVIC_SetPriority(SVCall_IRQn, 0xff); /* Lowest possible priority */
}

/*
 * Delete the currect checkpoint used for restore in NVM
 * Can be used to do a clean boot (restart the program)
 */
__attribute__((noinline))
int checkpoint_delete(void)
{
    nvm_write_byte(CPCMND_DEL);
    char resp = nvm_read_byte();
    if (resp != CPCMND_ACK) {
        // Deletion failed
        // TODO: handle
        return 0;
    }
    return 1;
}

#if CP_RESTORE_DISABLE
__attribute__((noinline))
void pyrestore(void) {}
#else
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
#endif /* CP_RESTORE_DISABLE */


uint32_t checkpoint_skipped = 0; // skip count
uint32_t checkpoint_performed = 0; // checkpoint count

#if CP_CHECKPOINT_DISABLE
__attribute__((noinline))
int checkpoint(void) {return 0;}
#else
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
__attribute__((noinline))
int checkpoint(void)
{
    char resp;

    checkpoint_schedule_update();

    if (!checkpoint_is_pending()) {
        checkpoint_skipped++;
        return 0;
    }

    nvm_write_byte(CPCMND_REQUEST_CHECKPOINT);

    resp = nvm_read_byte();
    if (resp != CPCMND_ACK) {
        return 2; // TODO: make actual error handling
    }

    checkpoint_memory();

#if CP_REGISTERS
    checkpoint_registers();
#endif

#if GDB_LOG_CP
    gdb_break_me();
#endif

    // NB: restore point
    if (checkpoint_restored() == 0) {
        /* Normal operation */
        nvm_write_byte(CPCMND_CONTINUE);
    }

    /* remove the pending status */
    checkpoint_clear_pending();

    /* update the checkpoint scheduling */
    checkpoint_schedule_callback();

    checkpoint_performed++;

    return checkpoint_restored();
}
#endif /* CP_CHECKPOINT_DISABLE */


/* TODO: Disable ISR for this bit */
void checkpoint_set_pending(void)
{
    checkpoint_pending = 1;
}

void checkpoint_clear_pending(void)
{
    checkpoint_pending = 0;
}

int checkpoint_is_pending(void)
{
#if CP_IGNORE_PENDING
    return 1;
#else
    return checkpoint_pending;
#endif
}


/*
 * Checkpoint scheduling
 */

#define CPS_CHECKPOINT_EVERY_MS 50
extern volatile uint64_t ticks_ms;

volatile uint64_t ticks_ms_last = 0;

// Time based checkpoint schedule
void checkpoint_schedule_update(void)
{
    uint64_t ticks_ms_diff;

    if (checkpoint_is_pending()) {
        return;
    }

    ticks_ms_diff = ticks_ms - ticks_ms_last;
    if (ticks_ms_diff > CPS_CHECKPOINT_EVERY_MS) {
        checkpoint_set_pending();
#ifdef CP_PRINT_PENDING
        printf("\r\n[CPS] set pending ms: %ld\r\n", (long)ticks_ms);
#endif
    }
}

void checkpoint_schedule_callback(void)
{
#ifdef CP_PRINT_COMMIT
    uint64_t ticks_ms_diff;

    ticks_ms_diff = ticks_ms - ticks_ms_last;
    ticks_ms_last = ticks_ms;

    printf("\r\n[CPS] update ms: %ld [dms: %ld] [skip: %ld] [performed: %ld]\r\n", (long)ticks_ms_last, (long)ticks_ms_diff, checkpoint_skipped, checkpoint_performed);
#endif
}
