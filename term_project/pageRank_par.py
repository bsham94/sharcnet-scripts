# -*- coding: utf-8 -*-
import numpy as np
from mpi4py import MPI

# General settings/variables
num_sites = 15              # Number of sites
infile = 'toy_example.txt'  # File to read

txtarr = []
counts = [0] * num_sites    # Zeroed array to hold counts

with open(infile, 'r') as fp:
    line=fp.readline()
    prev_site = 0
    outlinks=[]
    while line:
        val = line.split()
        curr_site = int(val[0])
        counts[curr_site-1] += 1
        if curr_site > num_sites:
            num_sites = curr_site
        if int(val[1]) > num_sites:
            num_sites = int(val[1])
        
        if (curr_site > prev_site) and prev_site != 0:
            txtarr.append(outlinks)
            outlinks=[]
            
        if(curr_site > prev_site+1):
            txtarr.append([])
        prev_site = curr_site
        outlinks.append(int(val[1]))
        
        line = fp.readline()
    txtarr.append(outlinks)
    
    
    h = np.zeros((num_sites, num_sites))
    
    comm = MPI.COMM_WORLD
    my_rank = comm.Get_rank()
    print("my rank is",my_rank)
    processors = comm.Get_size()
    print("size is",processors)
    
	#the size of each chunk the processors are in charge of
    k = len(txtarr) # processors
	
    if my_rank == 0:
        arr_start = my_rank*k +1
    else:
        arr_start = my_rank*k
	
    arr_end = arr_start+k
	
    for i in range(arr_start,arr_end):
        if len(txtarr[i])!=0:
            prob = 1/len(txtarr[i])
        else:
            prob = 0
        for j in txtarr[i]:
            h[i][j-1] = prob
# Create hyperlink matrix from file
    

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