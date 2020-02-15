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

# TOBEORNO: [84, 79, 66, 69, 79, 82, 78, 79]
# TOBEORNOTTOBEORF: [84, 79, 66, 69, 79, 82, 78, 79, 84, 256, 258, 260, 70]
# TOBEORNOTTOBEORTOBEORNOTODFDTWER: [84, 79, 66, 69, 79, 82, 78, 79, 84, 256, 258, 260, 265, 259, 261, 263, 79, 68, 70, 68, 84, 87, 69, 82]
# TOBEORNOTTOBEORTOBEORNOTODFDTWERTOBEORNOTTOBEORTOBEORNOTODFDTWER: [84, 79, 66, 69, 79, 82, 78, 79, 84, 256, 258, 260, 265, 259, 261, 263, 79, 68, 70, 68, 84, 87, 69, 82, 268, 260, 262, 264, 257, 269, 280, 270, 256, 273, 275, 277, 82]

while True:
    Pin2.value = False
    Pin0.value = True
    for i in range(0,2):
        compressed = compress('TOBEORNOTTOBEORTOBEORNOTODFDTWERTOBEORNOTTOBEORTOBEORNOTODFDTWER')
    Pin0.value = False
    if compressed == [84, 79, 66, 69, 79, 82, 78, 79, 84, 256, 258, 260, 265, 259, 261, 263, 79, 68, 70, 68, 84, 87, 69, 82, 268, 260, 262, 264, 257, 269, 280, 270, 256, 273, 275, 277, 82]:
        Pin4.value = True
        time.sleep(0.2)
        Pin4.value = False
    Pin2.value = True
    time.sleep(1)
    