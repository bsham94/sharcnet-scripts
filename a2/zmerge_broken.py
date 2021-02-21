#!/usr/bin/env python3

from mpi4py import MPI
import math
import random


def merge_arrays(a, b):
    a.append(math.inf)
    b.append(math.inf)
    c_size = len(a) + len(b)
    c = []
    i = 0
    j = 0
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
    result = (n/p)
    number = 0
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

a = [1, 5, 15, 18, 19, 21,23,24,27,29,30,31,32,37,42,49]
#a = []
#b = []
b = [2, 3, 4, 13, 15, 19,20,22,28,29,38,41,42,43,48,49]
c = []

array_size = 16
elements_per_group = int(math.log(array_size, 2))
# Required amount of groups
groups = math.ceil(array_size/elements_per_group)
tag = 0                 # Tag for MPI
master_proc = 0         # Master process
last_proc = processors - 1    # Final process
#start = calculateRange(my_rank, n, size)
#end = calculateRange(my_rank + 1, n, size) - 1

# Randomly generate the arrays in master process
# We will send them to the slave processes
if my_rank == master_proc:
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

if my_rank == master_proc:
    #print(str(r) + " groups and " + str(size) + " processors")
    if processors > groups:
        #print("more processors than groups, ignoring everything above r")
        processors = groups
    #if groups == processors:
        #print("1 group per process")
    #else:
        #print("more groups than processors, allocation required")
        #groups_per_proc = math.ceil(groups / processors)
        # for p in range(size):
        #print("rank: {p}, start: {start}, end: {end}".format(p=p, start=(starts[p] * groups_per_proc), end=(ends[p] * groups_per_proc) + 1))

groups_per_proc = math.ceil(groups / processors)
while processors % groups_per_proc != 0 and processors != 1:
        processors = processors - 1
#print("my_rank {r} processors {p}".format(r=my_rank, p=processors))
if my_rank < processors:
    #print("rank: {p}, groups_per: {group}".format(p=my_rank, group=groups_per_proc))
    group_start = (my_rank * groups_per_proc)
    group_end = (my_rank * groups_per_proc) + (groups_per_proc)
    prev_b = 0
    for group in range(group_start, group_end):
        a_start = group * elements_per_group
        a_end = group * elements_per_group + (elements_per_group-1)
        if a_start >= array_size:
            exit()
        if a_end >= array_size:
            a_end = array_size - 1
        a_current = a[a_start:a_end + 1]
        #print(a_current)
        b_start = 0
        if group > 0:
            b_start = prev_b + 1
        b_end = binary_search(b, 0, len(b) - 1, a_current[len(a_current) - 1]) - 1
        if group == groups - 1:
            b_end = b_end + 1
        if b_end >= array_size:
            b_end = array_size - 1
        if b_end < 0:
            b_end = array_size - 1
        prev_b = b_end
        b_current = b[b_start:b_end + 1]
        c_current = merge_arrays(a_current, b_current)
        if my_rank == master_proc:
            c.extend(c_current)
        else:
            #print("Proc {p} Group {g} is sending!".format(p=my_rank, g=group))
            comm.send(c_current, dest=master_proc, tag=tag)
        #print(c_current)
        #print("rank: {p}, start: {start}, end: {end}".format(p=my_rank, start=a_start, end=a_end))
        print("rank: {p}, start: {start}, end: {end}".format(p=my_rank, start=b_start, end=b_end))

if my_rank == master_proc:
    #print("processors {procs} groups {groups}".format(procs=processors, groups=groups))
    for source in range(master_proc + 1, processors):
        for group in range(groups_per_proc):
            #print("Proc {p} Group {g} is receiving!".format(p=source, g=group))
            # Merge in each merged array from each source
            c_current = comm.recv(source=source, tag=tag)
            c.extend(c_current)
    print(a)
    print(b)
    print(c)
    print("C was of length " + str(len(c)))
    

#print("rank: {p}, start: {start}, end: {end}".format(p=my_rank, start=(g_start * groups_per_proc), end=(g_end * groups_per_proc)))
#print("rank: {p}, groups_per: {group}, gstart: {start}, gend: {end}".format(p=my_rank, start=g_start, end=g_end, group=groups_per_proc))
#print("my_rank: " + str(my_rank) + " start: " + str(start) + " end: " + str(end))
