import time
import board
import digitalio
import array
from teaandtechtime_fft import fft
from math import sin, pi

Pin0 = digitalio.DigitalInOut(board.D0)
Pin0.direction = digitalio.Direction.OUTPUT
Pin2 = digitalio.DigitalInOut(board.D2)
Pin2.direction = digitalio.Direction.OUTPUT
Pin4 = digitalio.DigitalInOut(board.D4)
Pin4.direction = digitalio.Direction.OUTPUT

fft_size = 8

#create basic data structure to hold samples
samples = array.array('f', [0] * fft_size)

#assign a sinusoid to the samples
frequency = 63  # Set this to the Hz of the tone you want to generate.
for i in range(fft_size):
    samples[i] = sin(pi * 2 * i / (fft_size/frequency))

#create complex samples
test_complex_samples = []
for n in range(fft_size):
    test_complex_samples.append(((float(samples[n]))-1 + 0.0j))


# 8: round(abs(test_fft[0])) == 8 and round(abs(test_fft[len(test_fft)-1])) == 4
# 16: round(abs(test_fft[0])) == 16 and round(abs(test_fft[len(test_fft)-1])) == 8
# 32: round(abs(test_fft[0])) == 32 and round(abs(test_fft[len(test_fft)-1])) == 16
# 64: round(abs(test_fft[0])) == 64 and round(abs(test_fft[len(test_fft)-1])) == 32




while True:
    Pin2.value = False
    Pin0.value = True
    for i in range(0,5):
        test_fft = fft(test_complex_samples)
    Pin0.value = False
    if (round(abs(test_fft[0])) == 8 and round(abs(test_fft[len(test_fft)-1])) == 4):
        print("yes")
        Pin4.value = True
        time.sleep(0.2)
        Pin4.value = False
    Pin2.value = True
    time.sleep(1)