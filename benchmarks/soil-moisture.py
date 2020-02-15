import time
import board
import digitalio
import analogio

Pin0 = digitalio.DigitalInOut(board.D0)
Pin0.direction = digitalio.Direction.OUTPUT
Pin2 = digitalio.DigitalInOut(board.D2)
Pin2.direction = digitalio.Direction.OUTPUT
Pin4 = digitalio.DigitalInOut(board.D4)
Pin4.direction = digitalio.Direction.OUTPUT

adc0 = analogio.AnalogIn(board.A0)


while True:
    Pin2.value = False
    Pin0.value = True
    Pin4.value = True # Sensor power ON
    for i in range (0, 10):
        adc0_value = adc0.value
        #print(adc0_value)
    Pin4.value = False  # Sensor power OFF
    Pin0.value = False
    Pin2.value = True
    time.sleep(0.8)