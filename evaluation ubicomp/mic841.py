import argparse


def calculate_thresholds(r1_0,r2_0,r3_0):
	vin_hi_calc = 1.24 * ((r1_0+r2_0+r3_0)/r3_0)
	vin_lo_calc = 1.24 * ((r1_0+r2_0+r3_0) / (r2_0+r3_0))
	print("hi=%f, lo=%f" %(vin_hi_calc, vin_lo_calc))
	

def calculate_resistors(vin_hi, vin_low, target_total):
	r3=(1.24 * target_total) / vin_hi
	r2=(1.24 * target_total) / vin_low - r3
	r1 = target_total - r2 - r3
	print("r1=%d, r2=%d, r3=%d" %(r1, r2, r3))
	return [r1, r2, r3]


arr = calculate_resistors(4.0, 3, 2200000)

calculate_thresholds(arr[0], arr[1], arr[2])


# Stefany BFree shield 4.08 - 3.01
calculate_thresholds(1200000.00, 220000.00, 620000.00) #125, 224, 624

# Abu Bakar BFree shield 3.92 - 2.80
calculate_thresholds(1200000.00, 270000.00, 680000.00) #125, 274, 684