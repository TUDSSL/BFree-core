import time
import board
import digitalio

Pin0 = digitalio.DigitalInOut(board.D0)
Pin0.direction = digitalio.Direction.OUTPUT
Pin2 = digitalio.DigitalInOut(board.D2)
Pin2.direction = digitalio.Direction.OUTPUT
Pin4 = digitalio.DigitalInOut(board.D4)
Pin4.direction = digitalio.Direction.OUTPUT

def compress(uncompressed):

    # Build the dictionary.
    dict_size = 256
    dictionary = dict((chr(i), i) for i in range(dict_size))
    # in Python 3: dictionary = {chr(i): i for i in range(dict_size)}

    w = ""
    result = []
    for c in uncompressed:
        wc = w + c
        if wc in dictionary:
            w = wc
        else:
            result.append(dictionary[w])
            # Add wc to the dictionary.
            dictionary[wc] = dict_size
            dict_size += 1
            w = c

    # Output the code for w.
    if w:
        result.append(dictionary[w])
    return result

TOBEORNOTTOBEORF = [84, 79, 66, 69, 79, 82, 78, 79, 84, 256, 258, 260, 70]
TOBEORNOTTOBEORTOBEORNOTODFDTWER = [84, 79, 66, 69, 79, 82, 78, 79, 84, 256, 258, 260, 265, 259, 261, 263, 79, 68, 70, 68, 84, 87, 69, 82]
TOBEORNOTTOBEORTOBEORNOTODFDTWERTOBEORNOTTOBEORTOBEORNOTODFDTWER = [84, 79, 66, 69, 79, 82, 78, 79, 84, 256, 258, 260, 265, 259, 261, 263, 79, 68, 70, 68, 84, 87, 69, 82, 268, 260, 262, 264, 257, 269, 280, 270, 256, 273, 275, 277, 82]

it = 0
wrong_cnt = 0

# Tests
while True:
    if wrong_cnt == 0:
        Pin0.value = False
    else:
        print('Wong count:', wrong_cnt)
        print('Iteration:', it)
        Pin0.value = True

    # Start pusle
    Pin2.value = True
    Pin2.value = False

    it = it + 1
    #print('Iteration:', it)
    #print('Wong count:', wrong_cnt)

    compressed = compress('TOBEORNOTTOBEORF')
    if compressed != TOBEORNOTTOBEORF:
        wrong_cnt = wrong_cnt + 1
        print('Incorrect!')

    compressed = compress('TOBEORNOTTOBEORTOBEORNOTODFDTWER')
    if compressed != TOBEORNOTTOBEORTOBEORNOTODFDTWER:
        wrong_cnt = wrong_cnt + 1
        print('Incorrect!')

    compressed = compress('TOBEORNOTTOBEORTOBEORNOTODFDTWERTOBEORNOTTOBEORTOBEORNOTODFDTWER')
    if compressed != TOBEORNOTTOBEORTOBEORNOTODFDTWERTOBEORNOTTOBEORTOBEORNOTODFDTWER:
        wrong_cnt = wrong_cnt + 1
        print('Incorrect!')

    #time.sleep(1)
