import re
import matplotlib.pyplot as plt
import numpy as np

def readData(iter, frame, fileName):
    array = [[0] * iter for i in range(frame)]
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

defaultFile = 'data/defaultTest_O3.txt'
arrayFile   = 'data/arrayOptimizedTest_O3.txt'
avxFile     = 'data/avxOptimizedTest_O3.txt'

MAX_ITER  = 100
MAX_FRAME = 100

OX = list(range(MAX_ITER))

defaultO3data = readData(MAX_ITER, MAX_FRAME, defaultFile)
arrayO3data   = readData(MAX_ITER, MAX_FRAME, arrayFile)
avxO3data     = readData(MAX_ITER, MAX_FRAME, avxFile)

means_DEF_O3, stds_DEF_O3 = computeStats(defaultO3data, MAX_ITER, MAX_FRAME)
means_ARR_O3, stds_ARR_O3 = computeStats(arrayO3data,   MAX_ITER, MAX_FRAME)
means_AVX_O3, stds_AVX_O3 = computeStats(avxO3data,     MAX_ITER, MAX_FRAME)

def print_stats(means, stds, name, frame_count):
    mean_time = np.mean(means)
    avg_std = np.mean(stds)
    sem = avg_std / np.sqrt(frame_count)
    print(f"[Statistics for {name}]")
    print(f"Average Execution Time: {mean_time:.2f} ± {sem:.2f} ticks (SEM)")
    print(f"Mean Standard Deviation: {avg_std:.2f} ticks")
    print(f"Relative Error: {(sem/mean_time)*100:.2f}%\n")

print_stats(means_DEF_O3, stds_DEF_O3, "Default Version (-O3)",          MAX_FRAME)
print_stats(means_ARR_O3, stds_ARR_O3, "Array Optimized Version (-O3)",  MAX_FRAME)
print_stats(means_AVX_O3, stds_AVX_O3, "AVX256 Optimized Version (-O3)", MAX_FRAME)

plt.figure(figsize=(30, 30))

plt.errorbar(OX, means_DEF_O3, yerr=stds_DEF_O3, label='Default Version (-O3)',          color='hotpink', fmt='o', capsize=5) 
plt.errorbar(OX, means_ARR_O3, yerr=stds_ARR_O3, label='Array Optimized Version (-O3)',  color='orange',  fmt='o', capsize=5) 
plt.errorbar(OX, means_AVX_O3, yerr=stds_AVX_O3, label='AVX256 Optimized Version (-O3)', color='green',   fmt='o', capsize=5) 

plt.title(f'Performance comparison of naive, array optimized, and AVX256\noptimized Mandelbrot set rendering implementations (-O3)', fontsize=32)

plt.legend(fontsize = 32)
plt.grid()

plt.xlabel('Iteration Number', fontsize = 32)
plt.ylabel('Time, ticks',      fontsize = 32)

allMeans = means_DEF_O3 + means_ARR_O3 + means_AVX_O3
allStds =  stds_DEF_O3  + stds_ARR_O3  + stds_AVX_O3

minValue = min([m - s for m, s in zip(allMeans, allStds)])
maxValue = max([m + s for m, s in zip(allMeans, allStds)])

step = 5000000

plt.yticks(np.arange(np.floor(minValue / step) * step, np.ceil(maxValue / step) * step, step))

ax = plt.gca()
ax.tick_params(labelsize = 32)
ax.yaxis.get_offset_text().set_fontsize(32)

plt.savefig('img/totalCmp.png')