from mpi4py import MPI
import numpy as np
import math
import random
import time


def calculate_range(rank, n, p):
    # Pigeonhole principal
    result = (n/p)
    number = 0
    # min(rank, mod(n,p)) is added to range
    if(rank < (n % p)):
        number = rank
    else:
        number = n % p
    return int((rank * result) + number)


# Start MPI
comm = MPI.COMM_WORLD
# Find process rank
my_rank = comm.Get_rank()
# Find number of processes
p = comm.Get_size()

n = 28143
#n = 56

file = open("ens.matrix", 'r')
#file = open("toy_example.txt", 'r')

lines = file.readlines()


lower = calculate_range(my_rank, n, p)
upper = calculate_range(my_rank + 1, n, p)
array = np.zeros(((upper - lower), n), float)


dangle = float(1/n)
a = 0.85
ee = (1-a)*(dangle*1)

# Build each process matrix from file and Calculate Google Matrix
# G = aS + (1-a)(1/n(ee^t))
for line in lines:
    l = line.split()
    if len(l) == 3:
        if int(l[0]) < upper and int(l[0]) >= lower:
            if len(l[0]) != 0 and len(l[1]) != 0 and len(l[2]) != 0:
                if float(l[2]) == -1.0:
                    value = dangle * a + ee
                    array[int(l[0])-lower] = np.full((1, n), value, float)
                else:
                    array[int(l[0])-lower][int(l[1])] = (float(l[2]) * a) + ee

# Ax = b
# Each process calculates (upper - lower) amount values of the b vector and broadcasts its to the other processes
# When calculations are complete, each process should stop and wait for others to complete.
# When all process are done, each process should reciece all values from the others and build the new x vector.
