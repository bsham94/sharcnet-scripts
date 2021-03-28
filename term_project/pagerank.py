#!/usr/bin/env python3

from mpi4py import MPI
import numpy as np

# Start MPI
comm = MPI.COMM_WORLD
# Find process rank
my_rank = comm.Get_rank()
# Find number of processes
processors = comm.Get_size()

tag = 0                     # Tag for MPI
master_proc = 0             # Master process
last_proc = processors - 1  # Final process
num_sites = 15              # Number of sites
infile = 'toy_example.txt'  # File to read


counts = [0] * num_sites    # Zeroed array to hold counts
prev_site = -1              # Previous site
outlinks = []               # Outlink from previous site

# Create hyperlink matrix from file
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
print(h)
np.savetxt('out.csv', h, delimiter=",", fmt='%1.2f')