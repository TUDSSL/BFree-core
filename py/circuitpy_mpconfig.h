/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Dan Halbert for Adafruit Industries
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

// This file contains settings that are common across CircuitPython ports, to make
// sure that the same feature set and settings are used, such as in atmel-samd
// and nrf.

#include <stdint.h>

#ifndef __INCLUDED_MPCONFIG_CIRCUITPY_H
#define __INCLUDED_MPCONFIG_CIRCUITPY_H

// This is CircuitPython.
#define CIRCUITPY 1

// REPR_C encodes qstrs, 31-bit ints, and 30-bit floats in a single 32-bit word.
#define MICROPY_OBJ_REPR            (MICROPY_OBJ_REPR_C)

// options to control how MicroPython is built
// TODO(tannewt): Reduce this number if we want the REPL to function under 512
// free bytes.
// #define MICROPY_ALLOC_PARSE_RULE_INIT (64)

// Sorted alphabetically for easy finding.
//
// default is 128; consider raising to reduce fragmentation.
#define MICROPY_ALLOC_PARSE_CHUNK_INIT   (16)
// default is 512.
#define MICROPY_ALLOC_PATH_MAX           (256)
#define MICROPY_CAN_OVERRIDE_BUILTINS    (1)
#define MICROPY_COMP_CONST               (1)
#define MICROPY_COMP_DOUBLE_TUPLE_ASSIGN (1)
#define MICROPY_COMP_MODULE_CONST        (1)
#define MICROPY_COMP_TRIPLE_TUPLE_ASSIGN (0)
#define MICROPY_DEBUG_PRINTERS           (0)
//#define MICROPY_DEBUG_PRINTERS           (1)
#define MICROPY_EMIT_INLINE_THUMB        (0)
#define MICROPY_EMIT_THUMB               (0)
#define MICROPY_EMIT_X64                 (0)
#define MICROPY_ENABLE_DOC_STRING        (0)
#define MICROPY_ENABLE_FINALISER         (1)
#define MICROPY_ENABLE_GC                (1)
#define MICROPY_ENABLE_SOURCE_LINE       (1)
#define MICROPY_ERROR_REPORTING          (MICROPY_ERROR_REPORTING_NORMAL)
#define MICROPY_FLOAT_HIGH_QUALITY_HASH  (0)
#define MICROPY_FLOAT_IMPL               (MICROPY_FLOAT_IMPL_FLOAT)
#define MICROPY_GC_ALLOC_THRESHOLD       (0)
#define MICROPY_HELPER_LEXER_UNIX        (0)
#define MICROPY_HELPER_REPL              (1)
#define MICROPY_KBD_EXCEPTION            (1)
#define MICROPY_MEM_STATS                (0)
#define MICROPY_NONSTANDARD_TYPECODES    (0)
#define MICROPY_OPT_COMPUTED_GOTO        (1)
#define MICROPY_PERSISTENT_CODE_LOAD     (1)

#define MICROPY_PY_ARRAY                 (1)
#define MICROPY_PY_ARRAY_SLICE_ASSIGN    (1)
#define MICROPY_PY_ASYNC_AWAIT           (0)
#define MICROPY_PY_ATTRTUPLE             (1)

#define MICROPY_PY_BUILTINS_BYTEARRAY    (1)
#define MICROPY_PY_BUILTINS_ENUMERATE    (1)
#define MICROPY_PY_BUILTINS_FILTER       (1)
#define MICROPY_PY_BUILTINS_HELP         (1)
#define MICROPY_PY_BUILTINS_HELP_MODULES (1)
#define MICROPY_PY_BUILTINS_INPUT        (1)
#define MICROPY_PY_BUILTINS_MEMORYVIEW   (1)
#define MICROPY_PY_BUILTINS_MIN_MAX      (1)
#define MICROPY_PY_BUILTINS_PROPERTY     (1)
#define MICROPY_PY_BUILTINS_REVERSED     (1)
#define MICROPY_PY_BUILTINS_ROUND_INT    (1)
#define MICROPY_PY_BUILTINS_SET          (1)
#define MICROPY_PY_BUILTINS_SLICE        (1)
#define MICROPY_PY_BUILTINS_SLICE_ATTRS  (1)
#define MICROPY_PY_BUILTINS_STR_UNICODE  (1)

#define MICROPY_PY_CMATH                 (0)
#define MICROPY_PY_COLLECTIONS           (1)
#define MICROPY_PY_DESCRIPTORS           (1)
#define MICROPY_PY_IO_FILEIO             (1)
#define MICROPY_PY_GC                    (1)
// Supplanted by shared-bindings/math
#define MICROPY_PY_MATH                  (0)
#define MICROPY_PY_MICROPYTHON_MEM_INFO  (0)
// Supplanted by shared-bindings/struct
#define MICROPY_PY_STRUCT                (0)
#define MICROPY_PY_SYS                   (1)
#define MICROPY_PY_SYS_MAXSIZE           (1)
#define MICROPY_PY_SYS_STDFILES          (1)
// Supplanted by shared-bindings/random
#define MICROPY_PY_URANDOM               (0)
#define MICROPY_PY_URANDOM_EXTRA_FUNCS   (0)
#define MICROPY_PY___FILE__              (1)

#define MICROPY_QSTR_BYTES_IN_HASH       (1)
#define MICROPY_REPL_AUTO_INDENT         (1)
#define MICROPY_REPL_EVENT_DRIVEN        (0)
#define MICROPY_STACK_CHECK              (1)
#define MICROPY_STREAMS_NON_BLOCK        (1)
#define MICROPY_USE_INTERNAL_PRINTF      (1)

// fatfs configuration used in ffconf.h
//
// 1 = SFN/ANSI 437=LFN/U.S.(OEM)
#define MICROPY_FATFS_ENABLE_LFN      (1)
#define MICROPY_FATFS_LFN_CODE_PAGE   (437)
#define MICROPY_FATFS_USE_LABEL       (1)
#define MICROPY_FATFS_RPATH           (2)
#define MICROPY_FATFS_MULTI_PARTITION (1)

// Only enable this if you really need it. It allocates a byte cache of this size.
// #define MICROPY_FATFS_MAX_SS           (4096)

#define FILESYSTEM_BLOCK_SIZE       (512)

#define MICROPY_VFS                 (1)
#define MICROPY_VFS_FAT             (MICROPY_VFS)
#define MICROPY_READER_VFS          (MICROPY_VFS)


// type definitions for the specific machine

#define BYTES_PER_WORD (4)

#define MICROPY_MAKE_POINTER_CALLABLE(p) ((void*)((mp_uint_t)(p) | 1))

// Track stack usage. Expose results via ustack module.
#define MICROPY_MAX_STACK_USAGE       (0)

// This port is intended to be 32-bit, but unfortunately, int32_t for
// different targets may be defined in different ways - either as int
// or as long. This requires different printf formatting specifiers
// to print such value. So, we avoid int32_t and use int directly.
#define UINT_FMT "%u"
#define INT_FMT "%d"
typedef int mp_int_t; // must be pointer size
typedef unsigned mp_uint_t; // must be pointer size
typedef long mp_off_t;

#define MP_PLAT_PRINT_STRN(str, len) mp_hal_stdout_tx_strn_cooked(str, len)

#define mp_type_fileio mp_type_vfs_fat_fileio
#define mp_type_textio mp_type_vfs_fat_textio

#define mp_import_stat mp_vfs_import_stat
#define mp_builtin_open_obj mp_vfs_open_obj

// extra built in names to add to the global namespace
#define MICROPY_PORT_BUILTINS \
    { MP_OBJ_NEW_QSTR(MP_QSTR_help), (mp_obj_t)&mp_builtin_help_obj }, \
    { MP_OBJ_NEW_QSTR(MP_QSTR_input), (mp_obj_t)&mp_builtin_input_obj }, \
    { MP_OBJ_NEW_QSTR(MP_QSTR_open), (mp_obj_t)&mp_builtin_open_obj },

// board specific definitions
#include "mpconfigboard.h"

// CIRCUITPY_FULL_BUILD is defined in a *.mk file.

// Remove some lesser-used functionality to make small builds fit.
#define MICROPY_BUILTIN_METHOD_CHECK_SELF_ARG (CIRCUITPY_FULL_BUILD)
#define MICROPY_CPYTHON_COMPAT                (CIRCUITPY_FULL_BUILD)
#define MICROPY_MODULE_WEAK_LINKS             (CIRCUITPY_FULL_BUILD)
#define MICROPY_PY_ALL_SPECIAL_METHODS        (CIRCUITPY_FULL_BUILD)
#define MICROPY_PY_BUILTINS_COMPLEX           (CIRCUITPY_FULL_BUILD)
#define MICROPY_PY_BUILTINS_FROZENSET         (CIRCUITPY_FULL_BUILD)
#define MICROPY_PY_BUILTINS_STR_CENTER        (CIRCUITPY_FULL_BUILD)
#define MICROPY_PY_BUILTINS_STR_PARTITION     (CIRCUITPY_FULL_BUILD)
#define MICROPY_PY_BUILTINS_STR_SPLITLINES    (CIRCUITPY_FULL_BUILD)
#define MICROPY_PY_UERRNO                     (CIRCUITPY_FULL_BUILD)
// Opposite setting is deliberate.
#define MICROPY_PY_UERRNO_ERRORCODE           (!CIRCUITPY_FULL_BUILD)
#define MICROPY_PY_URE                        (CIRCUITPY_FULL_BUILD)
#define MICROPY_PY_URE_MATCH_GROUPS           (CIRCUITPY_FULL_BUILD)
#define MICROPY_PY_URE_MATCH_SPAN_START_END   (CIRCUITPY_FULL_BUILD)
#define MICROPY_PY_URE_SUB                    (CIRCUITPY_FULL_BUILD)

// LONGINT_IMPL_xxx are defined in the Makefile.
//
#ifdef LONGINT_IMPL_NONE
#define MICROPY_LONGINT_IMPL (MICROPY_LONGINT_IMPL_NONE)
#endif

#ifdef LONGINT_IMPL_MPZ
#define MICROPY_LONGINT_IMPL (MICROPY_LONGINT_IMPL_MPZ)
#define MP_SSIZE_MAX (0x7fffffff)
#endif

#ifdef LONGINT_IMPL_LONGLONG
#define MICROPY_LONGINT_IMPL (MICROPY_LONGINT_IMPL_LONGLONG)
#define MP_SSIZE_MAX (0x7fffffff)
#endif


// These CIRCUITPY_xxx values should all be defined in the *.mk files as being on or off.
// So if any are not defined in *.mk, they'll throw an error here.

#if CIRCUITPY_ANALOGIO
#define ANALOGIO_MODULE        { MP_OBJ_NEW_QSTR(MP_QSTR_analogio), (mp_obj_t)&analogio_module },
extern const struct _mp_obj_module_t analogio_module;
#else
#define ANALOGIO_MODULE
#endif

#if CIRCUITPY_AUDIOBUSIO
#define AUDIOBUSIO_MODULE        { MP_OBJ_NEW_QSTR(MP_QSTR_audiobusio), (mp_obj_t)&audiobusio_module },
extern const struct _mp_obj_module_t audiobusio_module;
#else
#define AUDIOBUSIO_MODULE
#endif

#if CIRCUITPY_AUDIOIO
#define AUDIOIO_MODULE         { MP_OBJ_NEW_QSTR(MP_QSTR_audioio), (mp_obj_t)&audioio_module },
extern const struct _mp_obj_module_t audioio_module;
#else
#define AUDIOIO_MODULE
#endif

#if CIRCUITPY_BITBANGIO
#define BITBANGIO_MODULE       { MP_OBJ_NEW_QSTR(MP_QSTR_bitbangio), (mp_obj_t)&bitbangio_module },
extern const struct _mp_obj_module_t bitbangio_module;
#else
#define BITBANGIO_MODULE
#endif

#if CIRCUITPY_BLEIO
#define BLEIO_MODULE           { MP_OBJ_NEW_QSTR(MP_QSTR_bleio), (mp_obj_t)&bleio_module },
extern const struct _mp_obj_module_t bleio_module;
#else
#define BLEIO_MODULE
#endif

#if CIRCUITPY_BOARD
#define BOARD_MODULE           { MP_OBJ_NEW_QSTR(MP_QSTR_board), (mp_obj_t)&board_module },
extern const struct _mp_obj_module_t board_module;

#define BOARD_I2C (defined(DEFAULT_I2C_BUS_SDA) && defined(DEFAULT_I2C_BUS_SCL))
#define BOARD_SPI (defined(DEFAULT_SPI_BUS_SCK) && defined(DEFAULT_SPI_BUS_MISO) && defined(DEFAULT_SPI_BUS_MOSI))
#define BOARD_UART (defined(DEFAULT_UART_BUS_RX) && defined(DEFAULT_UART_BUS_TX))

#if BOARD_I2C
#define BOARD_I2C_ROOT_POINTER mp_obj_t shared_i2c_bus;
#else
#define BOARD_I2C_ROOT_POINTER
#endif

// SPI is always allocated off the heap.

#if BOARD_UART
#define BOARD_UART_ROOT_POINTER mp_obj_t shared_uart_bus;
#else
#define BOARD_UART_ROOT_POINTER
#endif

#else
#define BOARD_MODULE
#define BOARD_I2C_ROOT_POINTER
#define BOARD_UART_ROOT_POINTER
#endif

#if CIRCUITPY_BUSIO
extern const struct _mp_obj_module_t busio_module;
#define BUSIO_MODULE           { MP_OBJ_NEW_QSTR(MP_QSTR_busio), (mp_obj_t)&busio_module },
#else
#define BUSIO_MODULE
#endif

#if CIRCUITPY_DIGITALIO
extern const struct _mp_obj_module_t digitalio_module;
#define DIGITALIO_MODULE       { MP_OBJ_NEW_QSTR(MP_QSTR_digitalio), (mp_obj_t)&digitalio_module },
#else
#define DIGITALIO_MODULE
#endif

#if CIRCUITPY_DISPLAYIO
extern const struct _mp_obj_module_t displayio_module;
extern const struct _mp_obj_module_t fontio_module;
extern const struct _mp_obj_module_t terminalio_module;
#define DISPLAYIO_MODULE       { MP_OBJ_NEW_QSTR(MP_QSTR_displayio), (mp_obj_t)&displayio_module },
#define FONTIO_MODULE       { MP_OBJ_NEW_QSTR(MP_QSTR_fontio), (mp_obj_t)&fontio_module },
#define TERMINALIO_MODULE      { MP_OBJ_NEW_QSTR(MP_QSTR_terminalio), (mp_obj_t)&terminalio_module },
#define CIRCUITPY_DISPLAY_LIMIT (3)
#else
#define DISPLAYIO_MODULE
#define FONTIO_MODULE
#define TERMINALIO_MODULE
#define CIRCUITPY_DISPLAY_LIMIT (0)
#endif

#if CIRCUITPY_FREQUENCYIO
extern const struct _mp_obj_module_t frequencyio_module;
#define FREQUENCYIO_MODULE       { MP_OBJ_NEW_QSTR(MP_QSTR_frequencyio), (mp_obj_t)&frequencyio_module },
#else
#define FREQUENCYIO_MODULE
#endif

#if CIRCUITPY_GAMEPAD
extern const struct _mp_obj_module_t gamepad_module;
#define GAMEPAD_MODULE         { MP_OBJ_NEW_QSTR(MP_QSTR_gamepad),(mp_obj_t)&gamepad_module },
#else
#define GAMEPAD_MODULE
#endif

#if CIRCUITPY_GAMEPADSHIFT
extern const struct _mp_obj_module_t gamepadshift_module;
#define GAMEPADSHIFT_MODULE         { MP_OBJ_NEW_QSTR(MP_QSTR_gamepadshift),(mp_obj_t)&gamepadshift_module },
#else
#define GAMEPADSHIFT_MODULE
#endif

#if CIRCUITPY_GAMEPAD || CIRCUITPY_GAMEPADSHIFT
// Scan gamepad every 32ms
#define CIRCUITPY_GAMEPAD_TICKS 0x1f
#define GAMEPAD_ROOT_POINTERS mp_obj_t gamepad_singleton;
#else
#define GAMEPAD_ROOT_POINTERS
#endif

#if CIRCUITPY_I2CSLAVE
extern const struct _mp_obj_module_t i2cslave_module;
#define I2CSLAVE_MODULE        { MP_OBJ_NEW_QSTR(MP_QSTR_i2cslave), (mp_obj_t)&i2cslave_module },
#else
#define I2CSLAVE_MODULE
#endif

#if CIRCUITPY_MATH
extern const struct _mp_obj_module_t math_module;
#define MATH_MODULE            { MP_OBJ_NEW_QSTR(MP_QSTR_math), (mp_obj_t)&math_module },
#else
#define MATH_MODULE
#endif

#if CIRCUITPY_MICROCONTROLLER
extern const struct _mp_obj_module_t microcontroller_module;
#define MICROCONTROLLER_MODULE { MP_OBJ_NEW_QSTR(MP_QSTR_microcontroller), (mp_obj_t)&microcontroller_module },
#else
#define MICROCONTROLLER_MODULE
#endif

#if CIRCUITPY_NEOPIXEL_WRITE
extern const struct _mp_obj_module_t neopixel_write_module;
#define NEOPIXEL_WRITE_MODULE  { MP_OBJ_NEW_QSTR(MP_QSTR_neopixel_write),(mp_obj_t)&neopixel_write_module },
#else
#define NEOPIXEL_WRITE_MODULE
#endif

#if CIRCUITPY_NETWORK
extern const struct _mp_obj_module_t network_module;
extern const struct _mp_obj_module_t socket_module;
#define NETWORK_MODULE         { MP_OBJ_NEW_QSTR(MP_QSTR_network), (mp_obj_t)&network_module },
#define SOCKET_MODULE          { MP_OBJ_NEW_QSTR(MP_QSTR_socket), (mp_obj_t)&socket_module },
#define NETWORK_ROOT_POINTERS mp_obj_list_t mod_network_nic_list;
#if MICROPY_PY_WIZNET5K
    extern const struct _mp_obj_module_t wiznet_module;
    #define WIZNET_MODULE        { MP_OBJ_NEW_QSTR(MP_QSTR_wiznet), (mp_obj_t)&wiznet_module },
#endif
#else
#define NETWORK_MODULE
#define SOCKET_MODULE
#define WIZNET_MODULE
#define NETWORK_ROOT_POINTERS
#endif

// This is not a top-level module; it's microcontroller.nvm.
#if CIRCUITPY_NVM
extern const struct _mp_obj_module_t nvm_module;
#endif

#if CIRCUITPY_OS
extern const struct _mp_obj_module_t os_module;
#define OS_MODULE              { MP_OBJ_NEW_QSTR(MP_QSTR_os), (mp_obj_t)&os_module },
#define OS_MODULE_ALT_NAME     { MP_OBJ_NEW_QSTR(MP_QSTR__os), (mp_obj_t)&os_module },
#else
#define OS_MODULE
#define OS_MODULE_ALT_NAME
#endif

#if CIRCUITPY_PEW
extern const struct _mp_obj_module_t pew_module;
#define PEW_MODULE          { MP_OBJ_NEW_QSTR(MP_QSTR__pew),(mp_obj_t)&pew_module },
#else
#define PEW_MODULE
#endif

#if CIRCUITPY_PIXELBUF
extern const struct _mp_obj_module_t pixelbuf_module;
#define PIXELBUF_MODULE        { MP_OBJ_NEW_QSTR(MP_QSTR__pixelbuf),(mp_obj_t)&pixelbuf_module },
#else
#define PIXELBUF_MODULE
#endif

#if CIRCUITPY_PULSEIO
extern const struct _mp_obj_module_t pulseio_module;
#define PULSEIO_MODULE         { MP_OBJ_NEW_QSTR(MP_QSTR_pulseio), (mp_obj_t)&pulseio_module },
#else
#define PULSEIO_MODULE
#endif

#if CIRCUITPY_PS2IO
extern const struct _mp_obj_module_t ps2io_module;
#define PS2IO_MODULE         { MP_OBJ_NEW_QSTR(MP_QSTR_ps2io), (mp_obj_t)&ps2io_module },
#else
#define PS2IO_MODULE
#endif

#if CIRCUITPY_RANDOM
extern const struct _mp_obj_module_t random_module;
#define RANDOM_MODULE          { MP_OBJ_NEW_QSTR(MP_QSTR_random), (mp_obj_t)&random_module },
#else
#define RANDOM_MODULE
#endif

#if CIRCUITPY_ROTARYIO
extern const struct _mp_obj_module_t rotaryio_module;
#define ROTARYIO_MODULE        { MP_OBJ_NEW_QSTR(MP_QSTR_rotaryio), (mp_obj_t)&rotaryio_module },
#else
#define ROTARYIO_MODULE
#endif

#if CIRCUITPY_RTC
extern const struct _mp_obj_module_t rtc_module;
#define RTC_MODULE             { MP_OBJ_NEW_QSTR(MP_QSTR_rtc), (mp_obj_t)&rtc_module },
#else
#define RTC_MODULE
#endif

#if CIRCUITPY_SAMD
extern const struct _mp_obj_module_t samd_module;
#define SAMD_MODULE            { MP_OBJ_NEW_QSTR(MP_QSTR_samd),(mp_obj_t)&samd_module },
#else
#define SAMD_MODULE
#endif

#if CIRCUITPY_STAGE
extern const struct _mp_obj_module_t stage_module;
#define STAGE_MODULE           { MP_OBJ_NEW_QSTR(MP_QSTR__stage), (mp_obj_t)&stage_module },
#else
#define STAGE_MODULE
#endif

#if CIRCUITPY_STORAGE
extern const struct _mp_obj_module_t storage_module;
#define STORAGE_MODULE         { MP_OBJ_NEW_QSTR(MP_QSTR_storage), (mp_obj_t)&storage_module },
#else
#define STORAGE_MODULE
#endif

#if CIRCUITPY_STRUCT
extern const struct _mp_obj_module_t struct_module;
#define STRUCT_MODULE          { MP_OBJ_NEW_QSTR(MP_QSTR_struct), (mp_obj_t)&struct_module },
#else
#define STRUCT_MODULE
#endif

#if CIRCUITPY_SUPERVISOR
extern const struct _mp_obj_module_t supervisor_module;
#define SUPERVISOR_MODULE      { MP_OBJ_NEW_QSTR(MP_QSTR_supervisor), (mp_obj_t)&supervisor_module },
#else
#define SUPERVISOR_MODULE
#endif

#if CIRCUITPY_TIME
extern const struct _mp_obj_module_t time_module;
#define TIME_MODULE            { MP_OBJ_NEW_QSTR(MP_QSTR_time), (mp_obj_t)&time_module },
#define TIME_MODULE_ALT_NAME   { MP_OBJ_NEW_QSTR(MP_QSTR__time), (mp_obj_t)&time_module },
#else
#define TIME_MODULE
#define TIME_MODULE_ALT_NAME
#endif

#if CIRCUITPY_TOUCHIO
extern const struct _mp_obj_module_t touchio_module;
#define TOUCHIO_MODULE         { MP_OBJ_NEW_QSTR(MP_QSTR_touchio), (mp_obj_t)&touchio_module },
#else
#define TOUCHIO_MODULE
#endif

#if CIRCUITPY_UHEAP
extern const struct _mp_obj_module_t uheap_module;
#define UHEAP_MODULE           { MP_OBJ_NEW_QSTR(MP_QSTR_uheap),(mp_obj_t)&uheap_module },
#else
#define UHEAP_MODULE
#endif

#if CIRCUITPY_USB_HID
extern const struct _mp_obj_module_t usb_hid_module;
#define USB_HID_MODULE         { MP_OBJ_NEW_QSTR(MP_QSTR_usb_hid),(mp_obj_t)&usb_hid_module },
#else
#define USB_HID_MODULE
#endif

#if CIRCUITPY_USB_MIDI
extern const struct _mp_obj_module_t usb_midi_module;
#define USB_MIDI_MODULE        { MP_OBJ_NEW_QSTR(MP_QSTR_usb_midi),(mp_obj_t)&usb_midi_module },
#else
#define USB_MIDI_MODULE
#endif

#if CIRCUITPY_USTACK
extern const struct _mp_obj_module_t ustack_module;
#define USTACK_MODULE          { MP_OBJ_NEW_QSTR(MP_QSTR_ustack),(mp_obj_t)&ustack_module },
#else
#define USTACK_MODULE
#endif

// These modules are not yet in shared-bindings, but we prefer the non-uxxx names.
#if MICROPY_PY_UERRNO
#define ERRNO_MODULE           { MP_ROM_QSTR(MP_QSTR_errno), MP_ROM_PTR(&mp_module_uerrno) },
#else
#define ERRNO_MODULE
#endif

#if MICROPY_PY_UJSON
#define JSON_MODULE            { MP_ROM_QSTR(MP_QSTR_json), MP_ROM_PTR(&mp_module_ujson) },
#else
#define JSON_MODULE
#endif

#if MICROPY_PY_URE
#define RE_MODULE { MP_ROM_QSTR(MP_QSTR_re), MP_ROM_PTR(&mp_module_ure) },
#else
#define RE_MODULE
#endif

// Define certain native modules with weak links so they can be replaced with Python
// implementations. This list may grow over time.
#define MICROPY_PORT_BUILTIN_MODULE_WEAK_LINKS \
    OS_MODULE \
    TIME_MODULE \

// Native modules that are weak links can be accessed directly
// by prepending their name with an underscore. This list should correspond to
// MICROPY_PORT_BUILTIN_MODULE_WEAK_LINKS, assuming you want the native modules
// to be accessible when overriden.
#define MICROPY_PORT_BUILTIN_MODULE_ALT_NAMES \
    OS_MODULE_ALT_NAME \
    TIME_MODULE_ALT_NAME \

// This is an inclusive list that should correspond to the CIRCUITPY_XXX list above,
// including dependencies such as TERMINALIO depending on DISPLAYIO (shown by indentation).
// Some of these definitions will be blank depending on what is turned on and off.
// Some are omitted because they're in MICROPY_PORT_BUILTIN_MODULE_WEAK_LINKS above.
#define MICROPY_PORT_BUILTIN_MODULES_STRONG_LINKS \
    ANALOGIO_MODULE \
    AUDIOBUSIO_MODULE \
    AUDIOIO_MODULE \
    BITBANGIO_MODULE \
    BLEIO_MODULE \
    BOARD_MODULE \
    BUSIO_MODULE \
    DIGITALIO_MODULE \
    DISPLAYIO_MODULE \
      FONTIO_MODULE \
      TERMINALIO_MODULE \
    ERRNO_MODULE \
    FREQUENCYIO_MODULE \
    GAMEPAD_MODULE \
    GAMEPADSHIFT_MODULE \
    I2CSLAVE_MODULE \
    JSON_MODULE \
    MATH_MODULE \
    MICROCONTROLLER_MODULE \
    NEOPIXEL_WRITE_MODULE \
    NETWORK_MODULE \
      SOCKET_MODULE \
      WIZNET_MODULE \
    PEW_MODULE \
    PIXELBUF_MODULE \
    PULSEIO_MODULE \
    PS2IO_MODULE \
    RANDOM_MODULE \
    RE_MODULE \
    ROTARYIO_MODULE \
    RTC_MODULE \
    SAMD_MODULE \
    STAGE_MODULE \
    STORAGE_MODULE \
    STRUCT_MODULE \
    SUPERVISOR_MODULE \
    TOUCHIO_MODULE \
    UHEAP_MODULE \
    USB_HID_MODULE \
    USB_MIDI_MODULE \
    USTACK_MODULE \

// If weak links are enabled, just include strong links in the main list of modules,
// and also include the underscore alternate names.
#if MICROPY_MODULE_WEAK_LINKS
#define MICROPY_PORT_BUILTIN_MODULES \
    MICROPY_PORT_BUILTIN_MODULES_STRONG_LINKS \
    MICROPY_PORT_BUILTIN_MODULE_ALT_NAMES
#else
// If weak links are disabled, included both strong and potentially weak lines
#define MICROPY_PORT_BUILTIN_MODULES \
    MICROPY_PORT_BUILTIN_MODULES_STRONG_LINKS \
    MICROPY_PORT_BUILTIN_MODULE_WEAK_LINKS
#endif

// We need to provide a declaration/definition of alloca()
#include <alloca.h>

#define MP_STATE_PORT MP_STATE_VM

#include "supervisor/flash_root_pointers.h"

#define CIRCUITPY_COMMON_ROOT_POINTERS \
    const char *readline_hist[8]; \
    vstr_t *repl_line; \
    mp_obj_t rtc_time_source; \
    GAMEPAD_ROOT_POINTERS \
    mp_obj_t pew_singleton; \
    mp_obj_t terminal_tilegrid_tiles; \
    BOARD_I2C_ROOT_POINTER \
    BOARD_UART_ROOT_POINTER \
    FLASH_ROOT_POINTERS \
    NETWORK_ROOT_POINTERS \

void run_background_tasks(void);

// TODO: Used in wiznet5k driver, but may not be needed in the long run.
#define MICROPY_THREAD_YIELD()

#define MICROPY_VM_HOOK_LOOP run_background_tasks();
#define MICROPY_VM_HOOK_RETURN run_background_tasks();

#define CIRCUITPY_AUTORELOAD_DELAY_MS 500
#define CIRCUITPY_FILESYSTEM_FLUSH_INTERVAL_MS 1000
#define CIRCUITPY_BOOT_OUTPUT_FILE "/boot_out.txt"

#endif  // __INCLUDED_MPCONFIG_CIRCUITPY_H
