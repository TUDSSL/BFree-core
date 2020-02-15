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

gs = gridspec.GridSpec(nrows=2, ncols=1, wspace=0, hspace=0.15, right = 0.99, left = 0.101, bottom = 0.16, top = 0.98, width_ratios=[5], height_ratios=[1, 1])
ax1 = fig.add_subplot(gs[0])
ax2 = fig.add_subplot(gs[1])


label_size = 8
ylim_1 = 0
ylim_2 = 1.3

xlim_1 = 0
xlim_2 = 5

vanilla = []
bfree = []

with open("/Users/abubakar/Desktop/Bfree Ubicomp/blink-vanilla-cpy.csv") as csvfile:
    readCSV = csv.reader(csvfile, delimiter=',')
    for row in readCSV: 
        vanilla.append(float(row[0]))

vanilla = [1 if value > 1 else 0 for value in vanilla]
time = [t/500.0 for t in range(0, len(vanilla), 1)]

ax1.plot(time, vanilla, '-', lw = 1.5, color = 'green')

ax1.set_ylim([ylim_1, ylim_2])
ax1.set_xlim([xlim_1, xlim_2])

ax1.set_xticks([])
ax1.set_yticks([0, 1])
ax1.set_yticklabels(('off', 'on'), fontsize=label_size)
ax1.set_title('Execution of Blink Code with a Period of 1.0sec', fontsize = label_size + 1)

with open("/Users/abubakar/Desktop/Bfree Ubicomp/blink-bfree-new.csv") as csvfile:
    readCSV = csv.reader(csvfile, delimiter=',')
    for row in readCSV: 
        bfree.append(float(row[0]))


bfree = [1 if value > 1 else 0 for value in bfree]
time = [t/500.0 for t in range(0, len(bfree), 1)]

ax2.plot(time, bfree, '-', lw = 1.5, color = 'blue')

ax2.set_ylim([ylim_1, ylim_2])
ax2.set_xlim([xlim_1, xlim_2])

ax2.set_xlabel('Time (sec)', fontsize = label_size)
ax2.set_ylabel('LED State', fontsize = label_size)
ax2.yaxis.set_label_coords(-0.08, 1)
# ax2.set_title('BFree CPy', fontsize = label_size + 1, fontweight = 'bold')

ax2.set_yticks([0, 1])
ax2.set_xticklabels(([0, 1, 2, 3, 4, 5]), fontsize=label_size)
ax2.set_yticklabels(('off', 'on'), fontsize=label_size)




legend_elements = [mpatches.Patch(color='green', label='Vanilla CPy', alpha = 1, linewidth = 0),
                    mpatches.Patch(color='blue', label='BFree CPy', alpha = 1, linewidth = 0)]
#                     mpatches.Patch(color='#826C5B', label='Port Init', alpha = 0.5, linewidth = 0),
#                     mpatches.Patch(color='#DDB100', label='Safe Mode', alpha = 1.0, linewidth = 0),
#                     mpatches.Patch(color='#00BFD3', label='File System', alpha = 0.6, linewidth = 0)
#                     # mpatches.Patch(color='#A53400', label='main() Search', alpha = 1.0, linewidth = 0),
#                     # mpatches.Patch(color='#EDC9AF', label='Compilation', alpha = 1.0, linewidth = 0),
#                     # mpatches.Patch(color='#F48B09', label='Before Execution', alpha = 1.0, linewidth = 0)
#                     ]


ax1.legend(handles=legend_elements, loc = 'upper right', ncol = 1, bbox_to_anchor=(1.004, 1.02), handletextpad=0.3, prop={'size':label_size - 1})


fig.set_size_inches(4, 1.5)
fig.savefig("blink-vanilla-vs-bfree-new.pdf", bbox_inches = 'tight')

fig.tight_layout()
plt.show()
