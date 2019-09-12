# This program uses circuit python,the 
# adafruit metro express m0 board,
# and the sparkfun Grid Eye Thermal Array Sensor 
# for human detection.
#program must include the following libraries 
import time
import busio
import board
import adafruit_amg88xx

i2c = busio.I2C(board.SCL, board.SDA)
amg = adafruit_amg88xx.AMG88XX(i2c)

while True:
    avg = 0
    for row in amg.pixels:
        # Pad to 1 decimal place
        print(['{0:.1f}'.format(temp) for temp in row])
        print("")
    for row in range(len(amg.pixels)):
        for col in range(len(amg.pixels)):
            avg += amg.pixels[row][col]
    total = avg/64
    #print("total is ",total)
    if total > 21.82: # temp threshold for human detection
        print("human detected, average is ", total)
    else:
        print("no human detected, average is ", total)
    time.sleep(1)