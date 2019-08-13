#include <stdint.h>

#include "nvm.h"
#include "mpy_comm.h"
#include "checkpoint_ctrl.h"

process_state_t ProcessState;

void process_error(process_state_t state)
{
    while (1) {
        // Crash
    }
}

// TODO: Can be made to a jump table if it all works
void process_dispatch(uint8_t byte)
{
    switch (ProcessState) {
        case (PS_COMMAND):
            process_command_byte(byte);
            break;
        case (PS_CHECKPOINT):
            process_checkpoint_command();
            break;
        case (PS_PRESTORE):
            process_restore_command();
            break;
        case (PS_ERROR):
        default:
            process_error(ProcessState);
    }
}

void process_command_byte(uint8_t byte)
{
    switch (byte) {
        case CPCMND_REQUEST_CHECKPOINT:
            mpy_write_byte(CPCMND_ACK);
            ProcessState = PS_CHECKPOINT;
            break;
        case CPCMND_REQUEST_RESTORE:
            //mpy_write_byte(CPCMND_ACK);
            ProcessState = PS_PRESTORE;
            break;
        default:
            // Unknown command
            process_error(byte);
    }
}

void process_checkpoint_command(void)
{
    uint8_t command;

    command = mpy_read_byte();
    switch(command) {
        case CPCMND_SEGMENT:
            process_checkpoint_segment();
            break;
        case CPCMND_REGISTERS:
            process_checkpoint_registers();
            break;
    }
}

void process_checkpoint_segment(void)
{
    segment_size_t addr_start, addr_end, size;

    mpy_read((char *)&addr_start, sizeof(segment_size_t));
    mpy_read((char *)&addr_end, sizeof(segment_size_t));

    /* Compute size */
    size = addr_end - addr_start;
    //TODO check if the size makes sense

    segment_t *segment = checkpoint_segment_alloc(size);
    segment->meta.type = SEGMENT_TYPE_MEMORY;
    segment->meta.size = size;
    segment->meta.addr_start = addr_start;
    segment->meta.addr_end = addr_end;

    mpy_write((char *)&size, sizeof(size)); // send size as ACK

    /* Now the segment data will be send */
    // TODO: add checkpoint management
    mpy_read_dma_blocking(segment->data, size);

    /* Send an ACK */
    mpy_write_byte(CPCMND_ACK);
}

void process_checkpoint_registers(void)
{
    registers_size_t size;

    mpy_read((char *)&size, sizeof(registers_size_t));

    segment_t *segment = checkpoint_segment_alloc(size);
    segment->meta.type = SEGMENT_TYPE_REGISTERS;
    segment->meta.size = size;

    /* Send an ACK */
    mpy_write_byte(CPCMND_ACK);

    /* Now the register data will be send */
    mpy_read_dma_blocking(segment->data, size);

    /* Send an ACK */
    mpy_write_byte(CPCMND_ACK);
}

void process_restore_command(void)
{
    segment_iter_t seg_iter;
    checkpoint_get_segment_iter(&seg_iter);

    while (seg_iter.segment != NULL) {
        if (seg_iter.segment->meta.type == SEGMENT_TYPE_MEMORY) {
            process_restore_segment(seg_iter.segment);
        } else if (seg_iter.segment->meta.type == SEGMENT_TYPE_REGISTERS) {
            process_restore_registers(seg_iter.segment);
        }
        checkpoint_get_next_segment(&seg_iter);
    }

    mpy_write_byte(CPCMND_CONTINUE);
}

void process_restore_segment(segment_t *segment)
{
    char resp;
    segment_size_t size;

    mpy_write_byte(CPCMND_SEGMENT);

    resp = mpy_read_byte();
    if (resp != CPCMND_ACK) {
        // Error
        return;
    }

    mpy_write((char *)&segment->meta.addr_start, sizeof(segment_size_t));
    mpy_write((char *)&segment->meta.addr_end, sizeof(segment_size_t));

    /* Wait for the size as ACK */
    mpy_read((char *)&size, sizeof(segment_size_t));

    if (size != segment->meta.size) {
        // Wrong size
        return;
    }

    /* Write the data stream */
    mpy_write_dma_blocking(segment->data, size);

    /* Wait for the ACK */
    resp = mpy_read_byte();

    if (resp != CPCMND_ACK) {
        // Error
        return;
    }
}

void process_restore_registers(segment_t *segment)
{
    char resp;

    mpy_write_byte(CPCMND_REGISTERS);

    resp = mpy_read_byte();
    if (resp != CPCMND_ACK) {
        // Error
        return;
    }

    /* Write the data stream */
    mpy_write_dma_blocking(segment->data, segment->meta.size);

    /* Wait for the ACK */
    resp = mpy_read_byte();
    if (resp != CPCMND_ACK) {
        // Error
        return;
    }
}

