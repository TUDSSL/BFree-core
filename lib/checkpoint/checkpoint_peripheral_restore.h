#ifndef MICROPY_INCLUDED_LIB_CHECKPOINT_CHECKPOINT_PERIPHERAL_RESTORE_H
#define MICROPY_INCLUDED_LIB_CHECKPOINT_CHECKPOINT_PERIPHERAL_RESTORE_H

extern void common_hal_busio_i2c_restore(void);
extern void common_hal_digitalio_digitalinout_restore(void);
extern void common_hal_analogio_analogin_restore(void);

static inline void checkpoint_peripheral_restore(void)
{
    common_hal_busio_i2c_restore();
    common_hal_analogio_analogin_restore();
    //common_hal_digitalio_digitalinout_restore();
}

#endif /* MICROPY_INCLUDED_LIB_CHECKPOINT_CHECKPOINT_PERIPHERAL_RESTORE_H */
