#include <stdint.h>
#include <stdio.h>

#include "nvm.h"
#include "mpy_comm.h"
#include "checkpoint_ctrl.h"

//#define DBG_PRINT(...)
#define DBG_PRINT printf

process_state_t ProcessState;

void process_error(process_state_t state)
{
    while (1) {
        char byte = mpy_read_byte();
        printf("Next byte: [0x%x]\r\n", byte);
    }
}

// TODO: Can be made to a jump table if it all works
void process_dispatch(void)
{
    char byte;
    DBG_PRINT("Process Dispatch\r\n");

    switch (ProcessState) {
        case (PS_COMMAND):
            byte = mpy_read_byte();
            DBG_PRINT("State: PC_COMMAND\r\n");
            process_command_byte(byte);
            break;
        case (PS_CHECKPOINT):
            DBG_PRINT("State: PC_CHECKPOINT\r\n");
            process_checkpoint_command();
            break;
        case (PS_PRESTORE):
            DBG_PRINT("State: PC_RESTORE\r\n");
            process_restore_command();
            break;
        case (PS_ERROR):
        default:
            DBG_PRINT("State: PC_RESTORE\r\n");
            process_error(ProcessState);
    }
}

void process_command_byte(uint8_t byte)
{
    DBG_PRINT("Process Command (%c [0x%x])\r\n", byte, (int)byte);

    switch (byte) {
        case CPCMND_REQUEST_CHECKPOINT:
            DBG_PRINT("Command: REQUEST_CHECKPOINT\r\n");
            mpy_write_byte(CPCMND_ACK);
            ProcessState = PS_CHECKPOINT;
            break;
        case CPCMND_REQUEST_RESTORE:
            DBG_PRINT("Command: REQUEST_RESTORE\r\n");
            //mpy_write_byte(CPCMND_ACK);
            ProcessState = PS_PRESTORE;
            break;
        default:
            // Unknown command
            //DBG_PRINT("Command: ERROR (%c [0x%x])\r\n", byte, (int)byte);
            DBG_PRINT("Command: ERROR\r\n");
            //process_error(byte);
    }
}

void process_checkpoint_command(void)
{
    uint8_t command;
    command = mpy_read_byte();

    DBG_PRINT("Process Checkpoint Command (%c, [0x%x])\r\n", command, (int)command);

    switch(command) {
        case CPCMND_SEGMENT:
            DBG_PRINT("CP Command: SEGMENT\r\n");
            process_checkpoint_segment();
            break;
        case CPCMND_REGISTERS:
            DBG_PRINT("CP Command: REGISTERS\r\n");
            process_checkpoint_registers();
            break;
        case CPCMND_CONTINUE:
            ProcessState = PS_COMMAND;
            checkpoint_commit();
            DBG_PRINT("CP Command: CONTINUE\r\n");
            break;
        default:
            DBG_PRINT("CP Command: ERROR\r\n");
            process_error(command);
    }
}

void process_checkpoint_segment(void)
{
    segment_size_t addr_start, addr_end, size;

    mpy_write_byte(CPCMND_ACK);
    mpy_read((char *)&addr_start, sizeof(segment_size_t));
    mpy_read((char *)&addr_end, sizeof(segment_size_t));

    /* Compute size */
    size = addr_end - addr_start;
    //TODO check if the size makes sense
    DBG_PRINT("SEGMENT: size = %d\r\n", (int)size);

    segment_t *segment = checkpoint_segment_alloc(size);
    segment->meta.type = SEGMENT_TYPE_MEMORY;
    segment->meta.size = size;
    segment->meta.addr_start = addr_start;
    segment->meta.addr_end = addr_end;

    mpy_write((char *)&size, sizeof(size)); // send size as ACK

    /* Now the segment data will be send */
    // TODO: add checkpoint management
    mpy_read_dma_blocking(segment->data, size);

#if 0
    DBG_PRINT("Segment data: \r\n");
    for (int i=0; i<size; i++) {
        DBG_PRINT("%x, ", segment->data[i]);
    }
    DBG_PRINT("\r\n");
#endif

    /* Send an ACK */
    mpy_write_byte(CPCMND_ACK);
}

void process_checkpoint_registers(void)
{
    registers_size_t size;

    mpy_read((char *)&size, sizeof(registers_size_t));

    DBG_PRINT("REGISTER: size = %d\r\n", (int)size);

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

    DBG_PRINT("Process Restore Command\r\n");

    while (seg_iter.segment != NULL) {
        if (seg_iter.segment->meta.type == SEGMENT_TYPE_MEMORY) {
            DBG_PRINT("REST Command: Segment\r\n");
            process_restore_segment(seg_iter.segment);
        } else if (seg_iter.segment->meta.type == SEGMENT_TYPE_REGISTERS) {
            DBG_PRINT("REST Command: Registers\r\n");
            process_restore_registers(seg_iter.segment);
        }
        checkpoint_get_next_segment(&seg_iter);
    }

    //DBG_PRINT("REST Command: SEND CONTINUE\r\n");
    //mpy_write_byte(CPCMND_CONTINUE);

    DBG_PRINT("Done with restore\r\n");
    ProcessState = PS_COMMAND;
}

void process_restore_segment(segment_t *segment)
{
    char resp;
    segment_size_t size;

    mpy_write_byte(CPCMND_SEGMENT);

    resp = mpy_read_byte();
    if (resp != CPCMND_ACK) {
        // Error
        printf("Restore segment: expected ACK got %x (%c)\r\n", (int)resp, resp);
        return;
    }

    mpy_write((char *)&segment->meta.addr_start, sizeof(segment_size_t));
    mpy_write((char *)&segment->meta.addr_end, sizeof(segment_size_t));

    /* Wait for the size as ACK */
    mpy_read((char *)&size, sizeof(segment_size_t));

    DBG_PRINT("Restore SEGMENT: size = %d\r\n", (int)size);

    if (size != segment->meta.size) {
        // Wrong size
        printf("Restore segment, wrong size. Got %ld expected %ld\r\n",
                size, segment->meta.size);
        return;
    }

#if 0
    DBG_PRINT("Segment data: \r\n");
    for (int i=0; i<size; i++) {
        DBG_PRINT("%x, ", segment->data[i]);
    }
    DBG_PRINT("\r\n");
#endif

    /* Write the data stream */
    mpy_write_dma_blocking(segment->data, size);

    DBG_PRINT("Restore segment: wait for ACK\r\n");

    /* Wait for the ACK */
    resp = mpy_read_byte();
    if (resp != CPCMND_ACK) {
        // Error
        printf("Restore segment: expected ACK after write got %x \r\n",
                (int)resp);
        process_error(0);
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

