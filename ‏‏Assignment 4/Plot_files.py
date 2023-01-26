#!/usr/bin/python
import numpy as np
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt

N = []
Z = []
M = []

# List of file names
file_name = 'job_'
# Iterate over the file names
for i in range(200):
    # Read file
    with open(file_name + str(i) + ".out.txt") as f:
        lines = f.readlines()
        for line in lines[:-1]:
            a, b, c = line.split(" - ")
            # build arrays
            N.append(int(a))
            Z.append(int(b))
            M.append(float(c))

# remove background
M = [np.nan if x == -1 else x for x in M]

# save plot
plt.scatter(N, Z, c=M)
plt.colorbar()
plt.savefig('Plot.png')

