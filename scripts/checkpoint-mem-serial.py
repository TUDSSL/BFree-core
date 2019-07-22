import serial
import time
import json
import time

from dataclasses import dataclass
from typing import List

PRINT_CHECKPOINT_JSON = False

#SERIAL_PORT='/dev/ttyACM0'
SERIAL_PORT='/dev/pts/13'
RESTORE_JSON_FILE='restore.json'

UNIQUE_CP_START_KEY = '##CHECKPOINT_START##\r\n'
UNIQUE_CP_END_KEY = '##CHECKPOINT_END##\r\n'

@dataclass
class Segment:
    addr_start: int
    addr_end: int
    data: List[int]

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
    print('Checkpoint data:', cp.decode(), end='')

    segments = []

    while len(cp) > 0:

        if cp[0] == b's'[0]:
            print('Parsing segment checkpoint')
            idx = cp[1:].decode().find(':')
            addr_start = cp[1:idx+1].decode()
            print('Start address: ' + addr_start)

            cp = cp[idx+2:]
            idx = cp.decode().find('\r\n')
            addr_end = cp[0:idx+1].decode()
            print('End address: ' + addr_end)

            cp = cp[idx+2:]

            # Next addr_end - addr_start bytes are the data
            addr_start = int(addr_start)
            addr_end = int(addr_end)

            size = addr_end - addr_start
            print('Data size:', size)
            print('Data:', cp[0:size])
            data = list(cp[0:size])
            print('Data list:', data)

            segments.append(Segment(addr_start, addr_end, data))
            cp = cp[size+2:] # add 2 for \r\n
            #print('remaining data:', cp)

        elif cp[0] == b'r'[0]:
            print('Parsing register checkpoint')

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
        return fn



def write_segments_to_json(filename, segments):
    json_data = {}
    json_data['restore'] = []
    json_data['restore']

    for segment in segments:
        json_segment = {}
        json_segment['addr_start'] = str(segment.addr_start)
        json_segment['addr_end'] = str(segment.addr_end)
        json_segment['data'] = segment.data
        json_data['restore'].append(json_segment)

    if PRINT_CHECKPOINT_JSON == True:
        j = json.dumps(json_data, indent=4, sort_keys=True)
        print(j)

    # Write json to file
    with open(filename, 'w') as outfile:
        json.dump(json_data, outfile)


ser = serial.Serial(SERIAL_PORT)  # open serial port
print('Opened serial port: ' + ser.name)


CheckpointFile = CheckpointFilename()
checkpoint_bytearray = bytearray()
checkpoint = False

while True:
    line_raw = ser.readline()
    line_str = line_raw.decode()
    clean_bytes = bytearray(line_raw)

    print(line_str, end='')

    if UNIQUE_CP_START_KEY in line_str:
        print('Found checkpoint start marker')
        checkpoint = True

        index_start = line_str.find(UNIQUE_CP_START_KEY)
        index_end = index_start + len(UNIQUE_CP_START_KEY)
        clean_bytes = clean_bytes[index_end:]

        print('Clean bytes START:', clean_bytes.decode())

        #line_bytearray.extend('test'.encode())
    if UNIQUE_CP_END_KEY in line_str:
        print('Found checkpoint end marker')
        checkpoint = False

        index_start = line_str.find(UNIQUE_CP_END_KEY)
        clean_bytes = clean_bytes[0:index_start]

        print('Clean bytes END:', clean_bytes.decode())

        checkpoint_bytearray.extend(clean_bytes)
        segments = parse_checkpoint(checkpoint_bytearray)
        write_segments_to_json(CheckpointFile.filename(), segments)
        checkpoint_bytearray = bytearray()

        #print(checkpoint_bytearray.decode())

    if checkpoint == True:
        checkpoint_bytearray.extend(clean_bytes)


