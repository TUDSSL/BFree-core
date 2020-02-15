import numpy as np
import matplotlib.pyplot as plt
from matplotlib.ticker import MaxNLocator
from matplotlib.ticker import ScalarFormatter
from collections import namedtuple
import matplotlib.gridspec as gridspec
from matplotlib.lines import Line2D
import matplotlib.patches as mpatches
from matplotlib.patches import Polygon
import csv


fig = plt.figure()

gs1 = gridspec.GridSpec(nrows=3, ncols=1, wspace=0, hspace=0.4, right = 0.99, left = 0.101, bottom = 0.16, top = 0.98, width_ratios=[5], height_ratios=[1, 1, 1])
ax1 = fig.add_subplot(gs1[0])
ax2 = fig.add_subplot(gs1[1])
ax3 = fig.add_subplot(gs1[2])

label_size = 8
ylim_1 = -1
ylim_2 = 16

xlim_1 = 5
xlim_2 = 85

time = []
board_status = []
sensor_status = []
cap_voltage = []

with open("/Users/abubakar/Desktop/Bfree Ubicomp/logic analyzer/bfree-solar-powered-soil-moisture-10-samples-per-second-analog.csv") as csvfile:
    readCSV = csv.reader(csvfile, delimiter=',')
    i = 0;
    for row in readCSV: 
        if i != 0:
            time.append(float(row[0]))
            board_status.append(float(row[1]))
            sensor_status.append(float(row[2]))
            cap_voltage.append(float(row[3]))
        i = 1

cap_voltage = [v+0.5 for v in cap_voltage]
board_status = [2.5 if v > 1 else 1.5 for v in board_status]
sensor_status = [1 if v > 1 else 0 for v in sensor_status]

ax1.plot(time, cap_voltage, '-', lw = 1.5, color = '#009E73', markevery=5, alpha = 1)
ax1.plot(time, board_status, '-', lw = 1.5, color = '#56B4E9', markevery=5, alpha = 1)
ax1.plot(time, sensor_status, '-', lw = 1.5, color = '#E69F00', markevery=5, alpha = 1)

# ax1.set_ylim([ylim_1, ylim_2])
ax1.set_xlim([xlim_1, xlim_2])

ax1.set_xticks([])
# ax1.set_xticklabels(([0, 20, 40, 60, 80]), fontsize=label_size)
ax1.set_yticks([0, 1, 1.5, 2.5, 3.5, 4.5, 5.5])
ax1.set_yticklabels((['OFF', 'ON', 'OFF', 'ON', 3.0, 4.0]), fontsize=label_size-1)
ax1.set_title('Solar-powered BFree', fontsize = label_size + 3, x=0.5, y=1.2)

# ax1.set_xlabel('Time (sec)', fontsize = label_size)
ax1.set_ylabel('Voltage (V)', fontsize = label_size-1)
ax1.yaxis.set_label_coords(-0.06, 0.78)
ax1.tick_params(labelsize = label_size)


time = []
board_status = []
sensor_status = []
cap_voltage = []

with open("/Users/abubakar/Desktop/Bfree Ubicomp/logic analyzer/cpy-solar-powered-soil-moisture-10-samples-per-second-analog.csv") as csvfile:
    readCSV = csv.reader(csvfile, delimiter=',')
    i = 0;
    for row in readCSV: 
        if i != 0:
            time.append(float(row[0]))
            board_status.append(float(row[1]))
            sensor_status.append(float(row[2]))
            cap_voltage.append(float(row[3]))
        i = 1

cap_voltage[22990:] = [4.02]*(len(cap_voltage) - len(cap_voltage[0:22990]) )
cap_voltage = [v+0.5 for v in cap_voltage]
board_status = [2.5 if v > 1 else 1.5 for v in board_status]
sensor_status = [1 if v > 1 else 0 for v in sensor_status]

ax2.plot(time, cap_voltage, '-', lw = 1.5, color = '#009E73', markevery=5, alpha = 1)
ax2.plot(time, board_status, '-', lw = 1.5, color = '#56B4E9', markevery=5, alpha = 1)
ax2.plot(time, sensor_status, '-', lw = 1.5, color = '#E69F00', markevery=5, alpha = 1)

# ax1.set_ylim([ylim_1, ylim_2])
ax2.set_xlim([xlim_1-2, xlim_2-2])

ax2.set_xticks([])
# ax2.set_xticklabels(([0, 20, 40, 60, 80]), fontsize=label_size)
ax2.set_yticks([0, 1, 1.5, 2.5, 3.5, 4.5, 5.5])
ax2.set_yticklabels((['OFF', 'ON', 'OFF', 'ON', 3.0, 4.0]), fontsize=label_size-1)
ax2.set_title('Solar-powered CircuitPython', fontsize = label_size + 3)

# ax2.set_xlabel('Time (sec)', fontsize = label_size)
ax2.set_ylabel('Voltage (V)', fontsize = label_size-1)
ax2.yaxis.set_label_coords(-0.06, 0.78)

ax2.tick_params(labelsize = label_size)

time = []
board_status = []
sensor_status = []
cap_voltage = []

with open("/Users/abubakar/Desktop/Bfree Ubicomp/logic analyzer/cpy-continuous-powered-soil-moisture-10-samples-per-second-analog.csv") as csvfile:
    readCSV = csv.reader(csvfile, delimiter=',')
    i = 0;
    for row in readCSV: 
        if i != 0:
            time.append(float(row[0]))
            board_status.append(float(row[1]))
            sensor_status.append(float(row[2]))
            cap_voltage.append(float(row[3]))
        i = 1

cap_voltage[22990:] = [4.02]*(len(cap_voltage) - len(cap_voltage[0:22990]) )
cap_voltage = [v+0.5 for v in cap_voltage]
board_status = [2.5 if v > 1 else 1.5 for v in board_status]
sensor_status = [1 if v > 1 else 0 for v in sensor_status]

ax3.plot(time, cap_voltage, '-', lw = 1.5, color = '#009E73', markevery=5, alpha = 1)
ax3.plot(time, board_status, '-', lw = 1.5, color = '#56B4E9', markevery=5, alpha = 1)
ax3.plot(time, sensor_status, '-', lw = 1.5, color = '#E69F00', markevery=5, alpha = 1)

# ax1.set_ylim([ylim_1, ylim_2])
ax3.set_xlim([xlim_1-2, xlim_2-2])

ax3.set_xticks([2, 22, 42, 62, 82])
ax3.set_xticklabels(([0, 20, 40, 60, 80]), fontsize=label_size-1)
ax3.set_yticks([0, 1, 1.5, 2.5, 3.5, 4.5, 5.5])
ax3.set_yticklabels((['OFF', 'ON', 'OFF', 'ON', 3.0, 4.0]), fontsize=label_size-1)
ax3.set_title('Battery-powered CircuitPython', fontsize = label_size + 3)

ax3.set_xlabel('Time (sec)', fontsize = label_size)
ax3.set_ylabel('Voltage (V)', fontsize = label_size-1)
ax3.yaxis.set_label_coords(-0.06, 0.78)

ax3.tick_params(labelsize = label_size)

legend_elements = [mpatches.Patch(color='#009E73', label='Capacitor voltage', alpha = 1, linewidth = 0),
                    mpatches.Patch(color='#56B4E9', label='Metro M0 + BFree', alpha = 1, linewidth = 0),
                    mpatches.Patch(color='#E69F00', label='Sensor reading trigger', alpha = 1.0, linewidth = 0)]
#                     mpatches.Patch(color='#DDB100', label='Safe Mode', alpha = 1.0, linewidth = 0),
#                     mpatches.Patch(color='#00BFD3', label='File System', alpha = 0.6, linewidth = 0)
#                     # mpatches.Patch(color='#A53400', label='main() Search', alpha = 1.0, linewidth = 0),
#                     # mpatches.Patch(color='#EDC9AF', label='Compilation', alpha = 1.0, linewidth = 0),
#                     # mpatches.Patch(color='#F48B09', label='Before Execution', alpha = 1.0, linewidth = 0)
#                     ]


ax1.legend(handles=legend_elements, loc = 'upper right', ncol = 3, bbox_to_anchor=(1.004, 1.3), handletextpad=0.3, prop={'size':label_size})


fig.set_size_inches(6, 4.5)
fig.savefig("cpy-bfree-soil-moisture-sensor.pdf", bbox_inches = 'tight')

fig.tight_layout()
plt.show()
