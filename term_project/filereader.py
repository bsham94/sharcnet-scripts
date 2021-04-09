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

n = 15
#n = 28143
# n = 56

file = open("lecture.txt", 'r')
#file = open("ens.matrix", 'r')
# file = open("toy_example.txt", 'r')

lines = file.readlines()

if my_rank == 0:
    lower = 0
    upper = calculate_range(my_rank + 1, n, p)
elif my_rank != 0 and my_rank != p - 1:
    lower = calculate_range(my_rank, n, p)
    upper = calculate_range(my_rank + 1, n, p)
else:
    lower = calculate_range(my_rank, n, p)
    upper = n

print(f'Rank: {my_rank} Lower: {lower} Upper: {upper}')

dangle = float(1/n)
a = 0.85
ee = (1-a)*(dangle*1)
print(ee)
array = np.full((n, n), ee, float)
for line in lines:
    l = line.split()
    if len(l) == 3:
        # if int(l[0]) < upper and int(l[0]) >= lower:
        if len(l[0]) != 0 and len(l[1]) != 0 and len(l[2]) != 0:
            if float(l[2]) == -1.0:
                array[int(l[0])] += dangle
            else:
                array[int(l[0])][int(l[1])] = (float(l[2]) * a) + ee
array = array.transpose()
# print(array)
#prev_r = np.full((n, 1), 0)
r = np.zeros((n, 1))
prev_r = np.full((n, 1), dangle, float)
count = 0
while not(np.array_equal(prev_r, r)) and count < 34:
    #prev_r = r.copy()
    for i in range(lower, upper):
        r[i] = np.dot(array[i], prev_r)
        # r = np.around(r.copy(), decimals=10)
    comm.Allgatherv(r[lower:upper], r)
    prev_r = np.around(r, decimals=10)
    count = count + 1
    if my_rank == 0:
        print(r)
        print(count)

# if my_rank == 0:
#     print(array)
#     for i in r:
#         print(i)

# # Ax = b
# # Each process calculates (upper - lower) amount values of the b vector and broadcasts its to the other processes
# # When calculations are complete, each process should stop and wait for others to complete.
# # When all process are done, each process should reciece all values from the others and build the new x vector.
# x = np.full((n, 1), dangle, float)
