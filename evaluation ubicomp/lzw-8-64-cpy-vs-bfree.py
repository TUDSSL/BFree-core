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
import statistics 


fig = plt.figure()

gs = gridspec.GridSpec(nrows=1, ncols=1, wspace=0, hspace=0.15, right = 0.99, left = 0.101, bottom = 0.16, top = 0.98, width_ratios=[5], height_ratios=[1])
ax1 = fig.add_subplot(gs[0])


label_size = 8
ylim_1 = 0
ylim_2 = 128

xlim_1 = 0
xlim_2 = 35

bar_width = 1

x = [

     # lzw 8
     [0.1305],
     [1000],
     [0.1885],
     [0.9730],

    # lzw 16
     [0.1445],
     [1000],
     [0.2],
     [1.397],
    
    # lzw 32
     [0.1715], 
     [1000],
     [0.226],
     [1.435],

    # lzw 64
     [0.2275],
     [1000],
     [0.3050],
     [1.9060],


     ]

# lzw 8
ax1.bar(1, statistics.mean(x[0]), bar_width,
                alpha=1, color='#E69F00', edgecolor = 'none')
ax1.plot(2, 0.00155, marker="X", color='red', alpha=0.7, markersize=6.5)
ax1.bar(3, statistics.mean(x[2]), bar_width,
                alpha=1, color='#56B4E9', edgecolor = 'none', hatch = '/./')
ax1.bar(4, statistics.mean(x[3]), bar_width,
                alpha=1, color='#009E73', edgecolor = 'none', hatch = '\\\\\\\\')

# lzw 16
ax1.bar(11, statistics.mean(x[4]), bar_width,
                alpha=1, color='#E69F00', edgecolor = 'none')
ax1.plot(12, 0.00155, marker="X", color='red', alpha=0.7, markersize=6.5)
ax1.bar(13, statistics.mean(x[6]), bar_width,
                alpha=1, color='#56B4E9', edgecolor = 'none', hatch = '/./')
ax1.bar(14, statistics.mean(x[7]), bar_width,
                alpha=1, color='#009E73', edgecolor = 'none', hatch = '\\\\\\\\')

# lzw 32
ax1.bar(21, statistics.mean(x[8]), bar_width,
                alpha=1, color='#E69F00', edgecolor = 'none')
ax1.plot(22, 0.00155, marker="X", color='red', alpha=0.7, markersize=6.5)
ax1.bar(23, statistics.mean(x[10]), bar_width,
                alpha=1, color='#56B4E9', edgecolor = 'none', hatch = '/./')
ax1.bar(24, statistics.mean(x[11]), bar_width,
                alpha=1, color='#009E73', edgecolor = 'none', hatch = '\\\\\\\\')

# lzw 64
ax1.bar(31, statistics.mean(x[8]), bar_width,
                alpha=1, color='#E69F00', edgecolor = 'none')
ax1.plot(32, 0.00155, marker="X", color='red', alpha=0.7, markersize=6.5)
ax1.bar(33, statistics.mean(x[10]), bar_width,
                alpha=1, color='#56B4E9', edgecolor = 'none', hatch = '/./')
ax1.bar(34, statistics.mean(x[11]), bar_width,
                alpha=1, color='#009E73', edgecolor = 'none', hatch = '\\\\\\\\')

ax1.set_yscale('log')
ax1.minorticks_off()

ax1.set_ylim([ylim_1, ylim_2])
ax1.set_xlim([xlim_1, xlim_2])

ax1.set_xlabel('String length (characters)', fontsize = label_size)
ax1.set_ylabel('Time (sec)', fontsize = label_size)
# ax1.set_title('Fibonacci Completion Time', fontsize = label_size + 1, fontweight = 'bold')
# # ax2.yaxis.set_label_coords(-0.08, 1)

ax1.set_xticks([2.5, 12.5, 22.5, 32.5])
ax1.set_xticklabels(([8, 16, 32, 64]), fontsize=label_size)
ax1.set_yticks([0.001, 0.01, 0.1, 1, 10, 100])
ax1.set_yticklabels(([0.001, 0.01, 0.1, 1, 10, 100]), fontsize=label_size)

plt.rcParams['hatch.linewidth'] = 0.3


legend_elements = [mpatches.Patch(facecolor='#E69F00', label='CPy (plugged-in)', alpha = 1, linewidth = 0),
                    Line2D([0], [0], marker='X', color='red', label='CPy (unplugged)', alpha = 0.7,  markerfacecolor='red', markersize=6.5, linestyle='None'),
                    mpatches.Patch(facecolor='#56B4E9', hatch = '//..//', label='BFree (plugged-in)', alpha = 1.0, linewidth = 0),
                    mpatches.Patch(facecolor='#009E73', hatch = '\\\\\\\\\\', label='BFree (unplugged)', alpha = 1.0, linewidth = 0)]
#                     mpatches.Patch(color='#00BFD3', label='File System', alpha = 0.6, linewidth = 0)
#                     # mpatches.Patch(color='#A53400', label='main() Search', alpha = 1.0, linewidth = 0),
#                     # mpatches.Patch(color='#EDC9AF', label='Compilation', alpha = 1.0, linewidth = 0),
#                     # mpatches.Patch(color='#F48B09', label='Before Execution', alpha = 1.0, linewidth = 0)
#                     ]


ax1.legend(handles=legend_elements, loc = 'upper left', ncol = 1, bbox_to_anchor=(0.004, 0.99), handletextpad=0.3, prop={'size':label_size - 1})


fig.set_size_inches(4, 2)
fig.savefig("lzw-8-64-cpy-vs-bfree.pdf", bbox_inches = 'tight')

fig.tight_layout()
plt.show()