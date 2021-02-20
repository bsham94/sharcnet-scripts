from mpi4py import MPI
import math
import random

# Binary search used to find the greatest index
# of B that is less than the last A of a process
def binary_search(a, start, end, value):
    if end >= start:
        mid = math.floor((start+end)/2)
        if a[mid] >= value:
            if (mid-1) >= 0 and a[mid-1] <= value:
                return mid
        if(a[mid]>value):
            return binary_search(a,start,mid -1, value)
        return binary_search(a,mid +1,end,value)
    return -1 # We reached the end without finding a B value

# Start MPI
comm = MPI.COMM_WORLD
# Find process rank
my_rank = comm.Get_rank()
# Find number of processes
size = comm.Get_size()

n = 128                 # Number of elements per array
k = int(math.log(n, 2)) # Number of a[] elements per processor
r = math.ceil(n/k)      # Number of processors required
tag = 0                 # Tag for MPI
masterProc = 0          # Master process
lastProc = size - 1

# If we don't have enough processes, abort
if size < r:
    # Only output from master process
    if my_rank == 0:
        print("[ ERROR ] Not enough processors allocated.")
        print("To parallel merge arrays of size " + str(n) +
              ", you must allocate at least " + str(r) + " processors.")
    quit()


a = []
b = []
# Randomly generate the arrays in master process
# We will send them to the slave processes
if my_rank == masterProc:
    # The random arrays must be generated sorted
    # To do so, we just generate 0-3 and append the previous
    for i in range(n):
        a_val = random.randint(0, 3)
        b_val = random.randint(0, 3)
        if i > 0:
            a_val = a_val + a[i-1]
            b_val = b_val + b[i-1]
        a.append(a_val)
        b.append(b_val)
    # Send to each slave process
    for source in range(1, size):
        comm.send(a, dest=source, tag=tag)
        comm.send(b, dest=source, tag=tag)
else:
    # Slave processes wait to receive the arrays
    a = comm.recv(source=masterProc, tag=tag)
    b = comm.recv(source=masterProc, tag=tag)

# Get the range for A
a_start = (my_rank) * k
a_end = (my_rank) * k + (k-1)
# Last proc should just go until the end
# Since it wont always have a full K elements
if my_rank == lastProc:
    a_end = n - 1

# Get the range for B
# Start will be sent to us if we are not master
# End is found through binary search
b_start = 0
b_end = binary_search(b, 0, len(b) - 1, a[a_end])

if my_rank == masterProc:
    # Master proc only needs to send it's end value
    comm.send(b_end, dest=(my_rank+1), tag=tag)
elif my_rank == lastProc:
    # Last proc only needs to receive it's start value
    b_start = comm.recv( source=(my_rank-1), tag=tag)
    # It runs until the end of B
    b_end = n
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
    b_end = n

# Decrement b_end so it doesn't overlap with another process
b_end = b_end - 1

# Calculate our array sizes
a_size = (a_end - a_start) + 1
b_size = (b_end - b_start) + 1
merge_size = a_size + b_size

# Crop A for our current process
a_new = a[a_start:a_end + 1]
# Only grab B elements if we were given a range
b_new = []
if b_start > -1 and b_end > -1:
    b_new = b[b_start:b_end + 1]
# Add infinity to signal end of array
# This helps with merging
a_new.append(math.inf)
b_new.append(math.inf)

merge = []
i = 0
j = 0
# Begin the merging in ascending order
for k in range(merge_size):
    if a_new[i] <= b_new[j]:
        # If both arrays are equally infinity
        if a_new[i] == math.inf and b_new[j] == math.inf:
            # Then we've reached the end
            break
        else:
            # Otherwise, append A
            # Since it's smaller or equal
            merge.append(a_new[i])
            i = i + 1
    else:
        # If B is smaller, append it
        merge.append(b_new[j])
        j = j + 1

if my_rank == 0:
    # Master proc outputs the arrays
    print("A:---------------------------------------------------------")
    print(a)
    print("B:---------------------------------------------------------")
    print(b)
    print("C:---------------------------------------------------------")
    c = []
    # Start with the master merge array
    c.extend(merge)
    for source in range(1, size):
        # Merge in each merged array from each source
        merge = comm.recv(source=source, tag=tag)
        c.extend(merge)
    # Output entire product
    print(c)
    print("------------------------------------------------------------")
    print("C was of length " + str(len(c)))
else:
    # Slave processes send their merge array to master proc
    comm.send(merge, dest=masterProc, tag=tag)
