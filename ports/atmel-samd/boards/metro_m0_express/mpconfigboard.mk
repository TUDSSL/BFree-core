LD_FILE = boards/samd21x18-bootloader-external-flash.ld
USB_VID = 0x239A
USB_PID = 0x8014
USB_PRODUCT = "Metro M0 Express"
USB_MANUFACTURER = "Adafruit Industries LLC"

CHIP_VARIANT = SAMD21G18A
CHIP_FAMILY = samd21

SPI_FLASH_FILESYSTEM = 1
EXTERNAL_FLASH_DEVICE_COUNT = 2
EXTERNAL_FLASH_DEVICES = "S25FL216K, GD25Q16C"
LONGINT_IMPL = MPZ
#INTERNAL_FLASH_FILESYSTEM = 1
#LONGINT_IMPL = NONE
#CIRCUITPY_SMALL_BUILD = 1

CFLAGS_INLINE_LIMIT = 60
SUPEROPT_GC = 0

CIRCUITPY_SMALL_BUILD = 1
CIRCUITPY_USB_MIDI = 0
