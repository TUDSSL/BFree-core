#ifndef MICROPY_INCLUDED_LIB_CHECKPOINT_CHECKPOINT_H
#define MICROPY_INCLUDED_LIB_CHECKPOINT_CHECKPOINT_H

typedef uint32_t segment_size_t;
typedef uint8_t registers_size_t;

void checkpoint_init(void);
void pyrestore(void);
int checkpoint(void);

// For debug
void nvm_write(char *src, size_t len);
void nvm_write_byte(char src_byte);
void nvm_read(char *dst, size_t len);
char nvm_read_byte(void);

#endif // MICROPY_INCLUDED_LIB_CHECKPOINT_CHECKPOINT_H
