#ifndef MICROPY_INCLUDED_LIB_CHECKPOINT_CHECKPOINT_H
#define MICROPY_INCLUDED_LIB_CHECKPOINT_CHECKPOINT_H

#define CHECKPOINT_SCHEDULE_UPDATE (1)
#define CHECKPOINT_PERIOD_MS 200
#define CHECKPOINT_HYBRID_PERIOD_MS 1000

enum checkpoint_schedule {
    CHECKPOINT_SCHEDULE_TIME,
    CHECKPOINT_SCHEDULE_TRIGGER,
    CHECKPOINT_SCHEDULE_HYBRID
};

struct checkpoint_config {
    enum checkpoint_schedule checkpoint_schedule;
    uint64_t cps_period_ms;             // Period of the checkpoints
    uint64_t cps_hybrid_period_ms;      // Period of the checkpoints before the trigger point
};

#define CHECKPOINT_CONFIG_DEFAULT { \
    .checkpoint_schedule = CHECKPOINT_SCHEDULE_TIME, \
    .cps_period_ms = CHECKPOINT_PERIOD_MS, \
    .cps_hybrid_period_ms = CHECKPOINT_HYBRID_PERIOD_MS, \
}

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

// For debug
void nvm_write(char *src, size_t len);
void nvm_write_byte(char src_byte);
void nvm_write_per_byte(char *src, size_t len);
void nvm_read(char *dst, size_t len);
char nvm_read_byte(void);

#endif // MICROPY_INCLUDED_LIB_CHECKPOINT_CHECKPOINT_H
