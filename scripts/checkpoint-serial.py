import serial
import time
import json
import time

from dataclasses import dataclass
from typing import List

PRINT_CHECKPOINT_JSON = False

SERIAL_PORT='/dev/ttyACM0'
#SERIAL_PORT='/dev/pts/13'
RESTORE_JSON_FILE='restore.json'

UNIQUE_CP_START_KEY = '##CHECKPOINT_START##\r\n'
UNIQUE_CP_END_KEY = '##CHECKPOINT_END##\r\n'

UNIQUE_RESTORE_START_KEY = '##CHECKPOINT_RESTORE##\r\n'

@dataclass
class Segment:
    addr_start: int
    addr_end: int
    data: List[int]


################################################################################
## Restoration
################################################################################

# Commands:
#   's' - restore memory segment
#   'c' - stop restoration, continue
#   'r' - restore registers (next step, is special)
#   'a' - ACK

# Serial state machine (1 restore)
#   P = python restore, M = metro board
#(0)    P -> M: send 's'                // request segment restore
#(0)    M -> P: send 'a'                // ack restore request
#(1)    P -> M: $addr_start:$addr_end
#(2)    M -> P: $size                   // ($addr_end-$addr_start)
#(3)    P -> M: $data[0:size]           // send all the data
#(4)    M -> P: $size                   // send size again as ack
#(5)    if another segment, goto (1)
#(7)    P -> M: 'c'                     // finished restore, continue
#(6)    DONE

def send_continue(serial):
    serial.write('c'.encode())

def write_registers(serial, data):
    print('Start register write')

    # Request to write the registers
    while True:
        print('Requesting register write')

        # Clear trash from the input
        serial.flushInput()

        serial.write('r'.encode())
        cb = serial.read(1)
        if cb == b'a':
            print('Ack received')
            break
        print('No or invalid response: ' + str(cb))
        time.sleep(1)

    # Send the registers
    print('Sending register data')
    serial.write(bytearray(data))
    serial.flush()
    print('Done sending data')

    # Check for a data ack
    print('Check for size ack')

    # Wait for size response to ACK
    size_str = serial.readline().decode()
    size_resp = int(size_str)
    if size_resp == 16*4:
       print('Size ack received')
    else:
        print('Invalid ack size received:', str(size_resp), 'expected:', segment_size)
        return


def write_segment(serial, addr_start, addr_end, data):

    if addr_start == addr_end == -1:
        print('Register checkpoint')
        write_registers(serial, data)
        return

    segment_size = addr_end - addr_start
    if segment_size < 1:
        print('Invalid segment size:', segment_size)
        return

    if len(data) != segment_size:
        print('Data size not equal to segment size (data size:',
                len(data), 'segment size:', segment_size)
        return

    print('Start segment write')

    # Request to write a segment
    while True:
        print('Requesting segment write')

        # Clear trash from the input
        serial.flushInput()

        serial.write('s'.encode())
        cb = serial.read(1)
        if cb == b'a':
            print('Ack received')
            break
        print('No or invalid response: ' + str(cb))
        time.sleep(1)

    # Write the address range
    segment_range_str = format(addr_start, 'x') + ':' + format(addr_end, 'x') + '\n'
    print('Writing segment: ' + segment_range_str, end='')
    serial.write(segment_range_str.encode())     # write a string
    serial.flush()

    # Wait for size response to ACK
    size_str = serial.readline()
    size_resp = int(size_str)
    if size_resp == segment_size:
       print('Size ack received')
    else:
        print('Invalid ack size received:', size_str, 'expected:', segment_size)
        return

    # Send the actual data
    print('Sending ' + str(segment_size) + ' bytes of data')
    serial.write(bytearray(data))
    serial.flush()
    print('Done sending data')

    # Check for a data ack
    print('Check for size ack')

    # Wait for size response to ACK
    size_str = serial.readline().decode()
    size_resp = int(size_str)
    if size_resp == segment_size:
       print('Size ack received')
    else:
        print('Invalid ack size received:', str(size_resp), 'expected:', segment_size)
        return


################################################################################
## Checkpointing
################################################################################

# Checkpoint to PC
#   M -> P: UNIQUE_CP_START_KEY     // signal the PC app that we wan't to send a checkpoint
#   // SEGMENT CP
#   M -> P: 's'                     // request to send a segment checkpoint (memory)
#   M -> P: $addr_start:$addr_end   // send the address range
#   M -> P: data[0:$size]           // send the checkpoint data
#   // REGISTER CP
#   M -> P: 'r'                     // request to send a register checkpoint
#   M -> P: $registers[0:15]        // send the register values
#   // END CP
#   M -> P: UNIQUE_CP_END_KEY       // Continue

def parse_checkpoint(cp):
    print('Parsing checkpoint')
    print('Checkpoint data:', cp, end='')

    segments = []

    while len(cp) > 0:

        if cp[0] == b's'[0]:
            print('Parsing segment checkpoint')
            idx = cp[1:].find(':'.encode())
            addr_start = cp[1:idx+1].decode()
            print('Start address: ' + addr_start)

            cp = cp[idx+2:]
            idx = cp.find('\r\n'.encode())
            addr_end = cp[0:idx+1].decode()
            print('End address: ' + addr_end)

            cp = cp[idx+2:]

            # Next addr_end - addr_start bytes are the data
            addr_start = int(addr_start, 16)
            addr_end = int(addr_end, 16)

            size = addr_end - addr_start
            print('Data size:', size)
            data_ba = cp[0:size]
            print('Data:', data_ba)
            data = list(data_ba)
            print('Data list:', data)

            segments.append(Segment(addr_start, addr_end, data))
            cp = cp[size+2:] # add 2 for \r\n
            print('remaining data:', cp)

        elif cp[0] == b'r'[0]:
            print('Parsing register checkpoint')
            cp = cp[1:]
            data = list(cp[0:16*4]) # Registers are 4 bytes each, 16 registers
            print('Data list (reg):', data)
            segments.append(Segment(-1, -1, data))

            cp = cp[16*4+2:] # add 2 for \r\n
            print('remaining data:', cp)

        else:
            print('Unknown command character: ' + str(cp[0]))
            return

    print('Segments:', segments)
    return segments


# Filename = checkpoint_epoch_checkpoint-count
class CheckpointFilename:
    base = 'checkpoint_'
    filename_base = ''
    checkpoint_cnt = 0
    last_checkpoint_file = ''

    def __init__(self, basename = ''):
        if basename == '':
            basename = self.base + str(int(time.time())) + '_'
        self.filename_base = basename

    def filename(self, cp_cnt = -1):
        if cp_cnt == -1:
            fn = self.filename_base + str(self.checkpoint_cnt) + '.json'
            self.checkpoint_cnt = self.checkpoint_cnt + 1
        else:
            fn = self.filename_base + str(cp_cnt) + '.json'
        self.last_checkpoint_file = fn
        return fn

    def last_filename(self):
        return self.last_checkpoint_file


def get_restore_segments(filename):
    segments = []
    with open(filename) as json_file:
        data = json.load(json_file)
        for segment in data['restore']:
            hex_mem = []
            for hex_mem_content in segment['data']:
                hex_mem.append(hex_mem_content)
            #print('Data:', hex_mem)
            print('Found restore entry for:', segment['addr_start'], '-', segment['addr_end'])
            addr_start = int(str(segment['addr_start']), 16)
            addr_end = int(str(segment['addr_end']), 16)
            segments.append(Segment(addr_start, addr_end, hex_mem))

    return segments

def write_segments_to_json(filename, segments):
    json_data = {}
    json_data['restore'] = []
    json_data['restore']

    for segment in segments:
        json_segment = {}
        json_segment['addr_start'] = format(segment.addr_start, 'x')
        json_segment['addr_end'] = format(segment.addr_end, 'x')
        json_segment['data'] = segment.data
        json_data['restore'].append(json_segment)

    if PRINT_CHECKPOINT_JSON == True:
        j = json.dumps(json_data, indent=4, sort_keys=True)
        print(j)

    # Write json to file
    with open(filename, 'w') as outfile:
        json.dump(json_data, outfile)


################################################################################
## Main loop
################################################################################

ser = serial.Serial(SERIAL_PORT)  # open serial port
print('Opened serial port: ' + ser.name)


CheckpointFile = CheckpointFilename()
checkpoint_bytearray = bytearray()
checkpoint = False

while True:
    line_raw = ser.readline()
    clean_bytes = bytearray(line_raw)

    # Print serial data (act like an output only serial terminal)
    print(line_raw, end='')
    #print(line_raw)

    if UNIQUE_CP_START_KEY.encode() in line_raw:
        print('Found checkpoint start marker')
        checkpoint = True

        index_start = line_raw.find(UNIQUE_CP_START_KEY.encode())
        index_end = index_start + len(UNIQUE_CP_START_KEY)
        clean_bytes = clean_bytes[index_end:]

        #print('Clean bytes START:', clean_bytes.decode())

        #line_bytearray.extend('test'.encode())
    if UNIQUE_CP_END_KEY.encode() in line_raw:
        print('Found checkpoint end marker')
        checkpoint = False

        index_start = line_raw.find(UNIQUE_CP_END_KEY.encode())
        clean_bytes = clean_bytes[0:index_start]

        #print('Clean bytes END:', clean_bytes.decode())

        checkpoint_bytearray.extend(clean_bytes)
        segments = parse_checkpoint(checkpoint_bytearray)
        write_segments_to_json(CheckpointFile.filename(), segments)
        checkpoint_bytearray = bytearray()

        #print(checkpoint_bytearray.decode())

    if checkpoint == True:
        checkpoint_bytearray.extend(clean_bytes)

    if UNIQUE_RESTORE_START_KEY.encode() in line_raw:
        print('Found restore request marker')
        fn = CheckpointFile.last_filename()
        if fn == '':
            print('No previous checkpoint, sending continue command')
            send_continue(ser)
        else:
            print('Restoring checkpoint from: ' + fn)
            segments = get_restore_segments(fn)
            print(segments) # debug

            # Retsore the segments
            for segment in segments:
                write_segment(ser, segment.addr_start, segment.addr_end, segment.data)
            send_continue(ser)

