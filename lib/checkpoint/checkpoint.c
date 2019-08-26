#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "py/mpconfig.h"

#include "supervisor/serial.h"
#include "py/mphal.h"
#include "supervisor/usb.h"
#include "tusb.h"

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
#include "py/mpconfig.h"

#include "lib/checkpoint/checkpoint.h"

#define CP_REGISTERS    (1)
#define CP_STACK        (1)
#define CP_DATA         (0)
#define CP_BSS          (0)
#define CP_GC           (0)

#if CP_GC
#include "py/runtime.h"
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

#define checkpoint_restored() checkpoint_svc_restore

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
 * NVM communication
 */
struct spi_m_sync_descriptor SPI_M_SERCOM1;
busio_spi_obj_t nv_spi_bus;
digitalio_digitalinout_obj_t cs_pin_nv;

digitalio_digitalinout_obj_t wr_pin_nv;

void nvm_comm_init(void) {
     // Init the CS pin for NV memory controller
    cs_pin_nv.base.type = &digitalio_digitalinout_type;
    common_hal_digitalio_digitalinout_construct(&cs_pin_nv, &pin_PA18);

    // NV Set CS high (disabled).
    common_hal_digitalio_digitalinout_switch_to_output(&cs_pin_nv, true, DRIVE_MODE_PUSH_PULL);
    common_hal_digitalio_digitalinout_never_reset(&cs_pin_nv);
    // SPI for NV comms
    nv_spi_bus.base.type = &busio_spi_type;
    nv_spi_bus.spi_desc = SPI_M_SERCOM1;
    common_hal_busio_spi_construct(&nv_spi_bus, &pin_PA17, &pin_PA16, &pin_PA19);
    common_hal_busio_spi_configure(&nv_spi_bus, 100000, 0, 0, 8);

    // Init the WR pin for NV memory controller
    wr_pin_nv.base.type = &digitalio_digitalinout_type;
    common_hal_digitalio_digitalinout_construct(&wr_pin_nv, &pin_PA07);
    common_hal_digitalio_digitalinout_switch_to_input(&wr_pin_nv, PULL_DOWN);
}

void nvm_write(char *src, size_t len) {
    while (!common_hal_busio_spi_try_lock(&nv_spi_bus));
    common_hal_digitalio_digitalinout_set_value(&cs_pin_nv, false);

    common_hal_busio_spi_write(&nv_spi_bus, (uint8_t *)src, len);

    common_hal_digitalio_digitalinout_set_value(&cs_pin_nv, true);
    common_hal_busio_spi_unlock(&nv_spi_bus);
}

void nvm_write_byte(char src_byte) {
    nvm_write(&src_byte, 1);
}

void nvm_read(char *dst, size_t len) {
    while (!common_hal_busio_spi_try_lock(&nv_spi_bus)) {}
    common_hal_digitalio_digitalinout_set_value(&cs_pin_nv, false);

    common_hal_busio_spi_read(&nv_spi_bus, (uint8_t *)dst, len, 0);

    common_hal_digitalio_digitalinout_set_value(&cs_pin_nv, true);
    common_hal_busio_spi_unlock(&nv_spi_bus);
}

char nvm_read_byte(void) {
    char dst_byte;
    nvm_read(&dst_byte, 1);

    return dst_byte;
}

#if 0
#define nvm_wait_process do {mp_hal_delay_ms(5);} while (0)

#define NVM_TRY_CNT 100
char nvm_wait_ack(void)
{
    char b;

    for (int i=0; i<NVM_TRY_CNT; i++) {
        b = nvm_read_byte();
        if (b == CPCMND_ACK) {
            return b;
        }
    }
    return b;
}
#endif

void nvm_wait_process(void) {
    // Wait untill the NVM signals it's ready
    while (common_hal_digitalio_digitalinout_get_value(&wr_pin_nv) == false);
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
        nvm_wait_process();
        nvm_write_byte(CPCMND_REGISTERS);

        // Send the register size
        registers_size_t register_size = sizeof(registers);
        nvm_wait_process();
        nvm_write((char *)&register_size, sizeof(registers_size_t));

        // Read ACK
        nvm_wait_process();
        resp = nvm_read_byte();
        if (resp != CPCMND_ACK) {
            return;
        }

        // Send the registers
        nvm_wait_process();
        nvm_write((char *)registers, register_size);

        // Read ACK
        nvm_wait_process();
        resp = nvm_read_byte();
        if (resp != CPCMND_ACK) {
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

    printf("%lx:%lx\r\n", addr_start, addr_end);

    nvm_wait_process();
    nvm_write_byte(CPCMND_SEGMENT);

    nvm_wait_process();
    resp = nvm_read_byte();
    if (resp != CPCMND_ACK) {
        return;
    }

    nvm_wait_process();
    nvm_write((char *)&addr_start, sizeof(segment_size_t));
    nvm_write((char *)&addr_end, sizeof(segment_size_t));

    nvm_wait_process();
    // Read the size as ACK
    nvm_read((char *)&segment_size, sizeof(segment_size_t));

    if (segment_size != size) {
        printf("Sizes do not match, ACK size: %ld, expected: %ld\r\n",
                segment_size, (segment_size_t)size);
        return;
    }

    // Send the data
    nvm_wait_process();
    nvm_write(start, size);

    // ACK
    nvm_wait_process();
    resp = nvm_read_byte();
    if (resp != CPCMND_ACK) {
        return;
    }
}

void checkpoint_memory(void) {

#if CP_STACK
    uint32_t *sp = _get_sp();
    size_t stack_size = (size_t)&_estack - (size_t)sp;
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

#if CP_GC
    char *gc_ptr_start = (char *)MP_STATE_MEM(gc_pool_start);
    char *gc_ptr_end = (char *)MP_STATE_MEM(gc_pool_end);
    if (gc_ptr_start != NULL && gc_ptr_end != NULL) {
        size_t gc_size = (size_t)gc_ptr_end - (size_t)gc_ptr_start;
        if (gc_size > 0) {
            checkpoint_memory_region(gc_ptr_start, gc_size);
        }
    }
#endif

    //printf(".stack\t[%p-%p,%d]\r\n", sp, &_estack, stack_size);
    //printf(".data\t[%p-%p,%d]\r\n", &_srelocate, &_erelocate, data_size);
    //printf(".bss\t[%p-%p,%d]\r\n", &_sbss, &_ebss, bss_size);
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
    segment_size_t addr_start, addr_end, size;

#if 0
    // Signal a restore (keep trying)
    while (1) {
        nvm_write_byte(CPCMND_REQUEST_RESTORE);
        mp_hal_delay_ms(10); // wait a bit for a response
        resp = nvm_read_byte();
        mp_hal_delay_ms(500);
    }
#endif

    // TODO: make retry
    nvm_wait_process();
    nvm_write_byte(CPCMND_REQUEST_RESTORE);

    while (1) {
        nvm_wait_process();
        int c = nvm_read_byte();
        int done = 0;

        switch (c) {
            case CPCMND_CONTINUE:
                done = 1;
                break;
            case CPCMND_REGISTERS:
                nvm_wait_process();
                nvm_write_byte(CPCMND_ACK); // send ACK

                nvm_wait_process();
                nvm_read((char *)&registers, sizeof(registers));

                nvm_wait_process();
                nvm_write_byte(CPCMND_ACK); // send ACK

                restore_registers();
                break;
            case CPCMND_SEGMENT:
                nvm_wait_process();
                nvm_write_byte(CPCMND_ACK); // send ACK

                nvm_wait_process();
                nvm_read((char *)&addr_start, sizeof(segment_size_t));
                nvm_read((char *)&addr_end, sizeof(segment_size_t));

                size = addr_end - addr_start;
                nvm_wait_process();
                nvm_write((char *)&size, sizeof(segment_size_t)); // send size as ACK

                // Now the memory segment writeback starts
                nvm_wait_process();
                nvm_read((char *)&addr_start, size);

                nvm_wait_process();
                nvm_write_byte(CPCMND_ACK); // send ACK

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

void checkpoint_init(void)
{
    nvm_comm_init();
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
char test_data[] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20};
int checkpoint(void)
{
    char resp;

    nvm_write_byte(CPCMND_REQUEST_CHECKPOINT);

    nvm_wait_process();
    resp = nvm_read_byte();
    if (resp != CPCMND_ACK) {
        return 2; // TODO: make actual error handling
    }

    checkpoint_memory_region(test_data, sizeof(test_data));
    //checkpoint_memory();

#if CP_REGISTERS
    checkpoint_registers();
#endif

    // NB: restore point
    if (checkpoint_restored() == 0) {
        /* Normal operation */
        nvm_wait_process();
        nvm_write_byte(CPCMND_CONTINUE);
    }

    return checkpoint_restored();
}
