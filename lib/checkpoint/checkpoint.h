#ifndef MICROPY_INCLUDED_LIB_CHECKPOINT_CHECKPOINT_H
#define MICROPY_INCLUDED_LIB_CHECKPOINT_CHECKPOINT_H

#define CHECKPOINT_SCHEDULE_UPDATE (1)
#define CHECKPOINT_PERIOD_MS 200
#define CHECKPOINT_PERIOD_MULTIPLIER 5 /* Used in combination with the HYBRID checkpoint scheduler */

/* Select only ONE */
#define CHECKPOINT_SCHEDULE_TIME_BASED                      (1)
#define CHECKPOINT_SCHEDULE_CAP_SIGNAL_BASED                (0)
#define CHECKPOINT_SCHEDULE_CAP_SIGNAL_TIME_HYBRID_BASED    (0)


typedef uint32_t segment_size_t;
typedef uint8_t registers_size_t;

void checkpoint_init(void);
int checkpoint_delete(void);
void pyrestore(void);
int checkpoint(void);
void nvm_reset(void);

// For bindings
void checkpoint_disable(void);
void checkpoint_enable(void);
bool checkpoint_enabled(void);

void checkpoint_set_pending(void);
void checkpoint_clear_pending(void);
int checkpoint_is_pending(void);

static inline void checkpoint_force(void) {
    checkpoint_set_pending();
    checkpoint();
}

void checkpoint_schedule_update(void);
void checkpoint_schedule_callback(void);

// For debug
void nvm_write(char *src, size_t len);
void nvm_write_byte(char src_byte);
void nvm_write_per_byte(char *src, size_t len);
void nvm_read(char *dst, size_t len);
char nvm_read_byte(void);

#endif // MICROPY_INCLUDED_LIB_CHECKPOINT_CHECKPOINT_H
