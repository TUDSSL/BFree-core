import serial
import time
import json

from dataclasses import dataclass
from typing import List

SERIAL_PORT='/dev/ttyACM0'
RESTORE_JSON_FILE='restore.json'

@dataclass
class Segment:
    addr_start: int
    addr_end: int
    data: List[int]

def get_restore_segments(filename=RESTORE_JSON_FILE):
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

def write_segment(serial, addr_start, addr_end, data):

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


segments = get_restore_segments()
print(segments)

ser = serial.Serial(SERIAL_PORT)  # open serial port
print('Opened serial port: ' + ser.name)

for segment in segments:
    write_segment(ser, segment.addr_start, segment.addr_end, segment.data)

#segment = segments[0]
#write_segment(ser, segment.addr_start, segment.addr_end, segment.data)

# End the restore
send_continue(ser)


# For debugging keep spitting out the serial data
while True:
    print(ser.readline().decode(), end='')


