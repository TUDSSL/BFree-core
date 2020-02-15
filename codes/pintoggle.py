import time
import board
import digitalio
import analogio
from simpleio import map_range

adc0 = analogio.AnalogIn(board.A0)
# adc1 = analogio.AnalogIn(board.A1)
# adc2 = analogio.AnalogIn(board.A2)
# adc3 = analogio.AnalogIn(board.A3)
# adc4 = analogio.AnalogIn(board.A4)
# adc5 = analogio.AnalogIn(board.A5)

# Pin0 = digitalio.DigitalInOut(board.D0)
# Pin0.direction = digitalio.Direction.OUTPUT

Pin2 = digitalio.DigitalInOut(board.D0)
Pin2.direction = digitalio.Direction.OUTPUT

Pin3 = digitalio.DigitalInOut(board.D2)
Pin3.direction = digitalio.Direction.OUTPUT

# Pin5 = digitalio.DigitalInOut(board.D5)
# Pin5.direction = digitalio.Direction.INPUT
# Pin5.pull = digitalio.Pull.UP
 
while True:
    Pin2.value = True
    Pin3.value = False
    time.sleep(0.5)
    Pin2.value = False
    Pin3.value = True
    time.sleep(0.5)

    # adc0_value = adc0.value
    # adc1_value = adc1.value
    # adc2_value = adc2.value
    # adc3_value = adc3.value
    # adc4_value = adc4.value
    # adc5_value = adc5.value

    # if adc0_value > 20000:
    #     Pin0.value = True
    # else:
    #     Pin0.value = False
    # print(adc5_value)

    # pin5_value = Pin5.value

    # if pin5_value == 1:
    #     Pin3.value = True
    # else:
    #     Pin3.value = False


