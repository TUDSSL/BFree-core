#ifndef MICROPY_INCLUDED_LIB_CHECKPOINT_CHECKPOINT_H
#define MICROPY_INCLUDED_LIB_CHECKPOINT_CHECKPOINT_H

typedef uint32_t segment_size_t;
typedef uint8_t registers_size_t;

void checkpoint_init(void);
void pyrestore(void);
int checkpoint(void);

#endif // MICROPY_INCLUDED_LIB_CHECKPOINT_CHECKPOINT_H
