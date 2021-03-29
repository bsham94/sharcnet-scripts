#!/usr/bin/env python3
import numpy as np

# General settings/variables
num_sites = 15              # Number of sites
infile = 'toy_example.txt'  # File to read


# Create hyperlink matrix from file
counts = [0] * num_sites    # Zeroed array to hold counts
prev_site = -1              # Previous site
outlinks = []               # Outlink from previous site
cur_site = 0                # Current site number
h = np.zeros((num_sites, num_sites))
with open(infile, 'r') as fp:
    # Read the first line
    line = fp.readline()
    while line:
        # get the current values
        val = line.split(' ')
        cur_site = int(val[0])
        cur_outlink = int(val[1])
        # Count how many times we've seen the current site
        counts[cur_site-1] += 1
        # Does it match the previous site we saw?
        if prev_site != cur_site:
            # No, then we're done with the previous site
            # (Assuming sites are given in order)
            if prev_site != -1:
                # Fill in the probability for each of it's outlinks
                prob = 1 / counts[prev_site-1]
                for link in outlinks:
                    h[prev_site-1][link-1] = prob
            # Make current the new previous site
            prev_site = cur_site
            # Reset the outlinks
            outlinks = []
        # Append the current outlink
        outlinks.append(cur_outlink)
        # Move to the next line
        line = fp.readline()
    prob = 1 / counts[cur_site - 1]
    for link in outlinks:
        h[cur_site-1][link-1] = prob

# Stochasticity adjustment
e = np.array([[1] for x in range(num_sites)])
a = np.zeros((num_sites, 1))
for i in range(0, num_sites):
    if counts[i] == 0:
        a[i] = 1
s = h + a * ((1/num_sites) * e.transpose())

# Primitivity adjustment
alpha = 0.85
bigE = e.dot(e.transpose())
g = (s * alpha) + (1-alpha)*((1/num_sites)*bigE)
r = np.array([[1/num_sites] for x in range(num_sites)])
prev_r = np.zeros((num_sites, 1))

# Iterate to calculate PageRank
iterations = 0
while not(np.array_equal(prev_r, r)) and i < 1000:
    prev_r = np.copy(r)
    r = np.dot(g.transpose(),np.copy(r))
    r = np.around(r.copy(), decimals=10)
    iterations += 1

sortedList = sorted(((v, i) for i, v in enumerate(r)), reverse=True)
print("Final PageRank vector:")
for i, (value, index) in enumerate(sortedList):
    print("#{0} -> Site {1}, PageRank {2}".format(i+1, index+1, value))
print("Converged in: " + str(iterations) + " iterations")
