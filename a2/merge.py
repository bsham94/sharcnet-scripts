#!/usr/bin/env python3

from mpi4py import MPI
import math
import random
import time


def merge_arrays(a, b):
    # Add infinity to signal end of array
    a.append(math.inf)
    b.append(math.inf)
    c_size = len(a) + len(b)
    c = []
    i = 0
    j = 0
    # Begin the merging in ascending order
    for x in range(c_size):
        if a[i] <= b[j]:
            # If both arrays are equally infinity
            if a[i] == math.inf and b[j] == math.inf:
                # Then we've reached the end
                break
            else:
                # Otherwise, append A
                # Since it's smaller or equal
                c.append(a[i])
                i = i + 1
        else:
            # If B is smaller, append it
            c.append(b[j])
            j = j + 1
    return c


def binary_search(a, start, end, value):
    if end >= start:
        mid = math.floor((start+end)/2)
        if a[mid] >= value:
            if (mid-1) >= 0 and a[mid-1] <= value:
                return mid
        if(a[mid] > value):
            return binary_search(a, start, mid - 1, value)
        return binary_search(a, mid + 1, end, value)
    return -1  # We reached the end without finding a B value


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
processors = comm.Get_size()

# Define our 3 arrays
a = []
b = []
c = []

array_size = 1000000000           # Size of arrays
tag = 0                     # Tag for MPI
master_proc = 0             # Master process
last_proc = processors - 1  # Final process

# Randomly generate the arrays in master process
# We will send them to the slave processes
if my_rank == master_proc:
    start_time = time.time()
    # The random arrays must be generated sorted
    # To do so, we just generate 0-3 and append the previous
    for i in range(array_size):
        a_val = random.randint(0, 3)
        b_val = random.randint(0, 3)
        if i > 0:
            a_val = a_val + a[i-1]
            b_val = b_val + b[i-1]
        a.append(a_val)
        b.append(b_val)
    # Send to each slave process
    for source in range(1, processors):
        comm.send(a, dest=source, tag=tag)
        comm.send(b, dest=source, tag=tag)
else:
    # Slave processes wait to receive the arrays
    a = comm.recv(source=master_proc, tag=tag)
    b = comm.recv(source=master_proc, tag=tag)

# Get the range for A
a_start = calculate_range(my_rank, array_size, processors)
a_end = calculate_range(my_rank + 1, array_size, processors) - 1

# Get the range for B
# Start will be sent to us if we are not master
# End is found through binary search
b_start = 0
b_end = binary_search(b, 0, len(b) - 1, a[a_end])

if my_rank == master_proc:
    # Master proc only needs to send it's end value
    comm.send(b_end, dest=(my_rank+1), tag=tag)
elif my_rank == last_proc:
    # Last proc only needs to receive it's start value
    b_start = comm.recv(source=(my_rank-1), tag=tag)
    # It runs until the end of B
    b_end = array_size
else:
    # Every other process receives it's start value
    b_start = comm.recv(source=(my_rank-1), tag=tag)
    # If it starts at -1, we've reached the end of B already
    if b_start < 0:
        # So send -1 to the next proc too
        b_end = -1
    comm.send(b_end, dest=(my_rank+1), tag=tag)

# Check if end was -1 and change it to full range
# Needed in cases where start is defined but end was not found
# If start is -1 too, then this won't matter
if b_end < 0:
    b_end = array_size

# Decrement b_end so it doesn't overlap with another process
b_end = b_end - 1

# Crop arrays from start and end
a_current = a[a_start:a_end + 1]
# Only grab B elements if we were given a range
b_current = []
if b_start > -1 and b_end > -1:
    b_current = b[b_start:b_end + 1]
# Run the merge
c_current = merge_arrays(a_current, b_current)

if my_rank == master_proc:
    # Master proc starts the c array
    c.extend(c_current)
else:
    # Other processes send to master
    comm.send(c_current, dest=master_proc, tag=tag)

if my_rank == master_proc:
    # Grab all waiting arrays and merge
    for source in range(master_proc + 1, processors):
        # Merge in each merged array from each source
        c_current = comm.recv(source=source, tag=tag)
        c.extend(c_current)
    print("{n} elements per array, computed on {p} processors.".format(n=array_size, p=processors))
    print("Completed in {:.3f} seconds.".format(time.time() - start_time))
    print("Index 0: {}".format(c[0]))
    print("Index 10: {}".format(c[10]))
    print("Index n/2: {}".format(c[int(len(c)/2 - 1)]))
    print("Index n-10: {}".format(c[int(len(c)-11)]))
    print("Index n: {}".format(c[int(len(c)-1)]))
    print("C was of length {}.".format(len(c)))
