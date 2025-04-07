import re
import matplotlib.pyplot as plt
import numpy as np

def readData(iter, frame, fileName):
    array = [[0] * iter for _ in range(frame)]
    with open(fileName, 'r') as file:
        for i in range(iter):
            for f in range(frame):
                line = list(map(int, re.split(r'[\s/]', file.readline().strip())))
                array[f][i] = line[2]
    return array

def computeStats(data, MAX_ITER, MAX_FRAME):
    means = []
    stds = []
    for i in range(MAX_ITER):
        values = [data[f][i] for f in range(MAX_FRAME)]
        means.append(np.mean(values))
        stds.append(np.std(values))
    return means, stds

def print_stats(means, stds, name, frame_count):
    mean_time = np.mean(means)
    avg_std = np.mean(stds)
    sem = avg_std / np.sqrt(frame_count)
    print(f"[Statistics for {name}]")
    print(f"Average Execution Time: {mean_time:.2f} ± {sem:.2f} ticks (SEM)")
    print(f"Mean Standard Deviation: {avg_std:.2f} ticks")
    print(f"Relative Error: {(sem/mean_time)*100:.2f}%\n")

defaultFileO0 = 'data/defaultTest_O0.txt'
defaultFileO3 = 'data/defaultTest_O3.txt'

MAX_ITER  = 100
MAX_FRAME = 100

OX = list(range(MAX_ITER))

defaultO0data = readData(MAX_ITER, MAX_FRAME, defaultFileO0)
defaultO3data = readData(MAX_ITER, MAX_FRAME, defaultFileO3)

means_O0, stds_O0 = computeStats(defaultO0data, MAX_ITER, MAX_FRAME)
means_O3, stds_O3 = computeStats(defaultO3data, MAX_ITER, MAX_FRAME)

print_stats(means_O0, stds_O0, "Default Version (-O0)", MAX_FRAME)
print_stats(means_O3, stds_O3, "Default Version (-O3)", MAX_FRAME)

plt.figure(figsize=(30, 30))

plt.errorbar(OX, means_O0, yerr=stds_O0, label='Default Version (-O0)', color='hotpink', fmt='o', capsize=5) 
plt.errorbar(OX, means_O3, yerr=stds_O3, label='Default Version (-O3)', color='green',   fmt='o', capsize=5)

# plt.title('Performance comparison of the naive version of the Mandelbrot set rendering\nwith different compiler options (-O0 and -O3)', fontsize=32)

# plt.legend(fontsize=32)
# plt.grid()

# plt.xlabel('Iteration Number', fontsize=32)
# plt.ylabel('Time, ticks', fontsize=32)

# allMeans = means_O0 + means_O3
# allStds  = stds_O0  + stds_O3
# minValue = min(m - s for m, s in zip(allMeans, allStds))
# maxValue = max(m + s for m, s in zip(allMeans, allStds))

# step = 7500000

# plt.yticks(np.arange(np.floor(minValue / step) * step, np.ceil(maxValue / step) * step, step))

# ax = plt.gca()
# ax.tick_params(labelsize = 32)
# ax.yaxis.get_offset_text().set_fontsize(32)

# plt.savefig('img/defaultCmp.png')