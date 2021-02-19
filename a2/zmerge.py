from mpi4py import MPI
import math
import random

def binary_search(a, start, end, value):
    if end >= start:
        mid = math.floor((start+end)/2)
        if a[mid] >= value:
            if (mid-1) >= 0 and a[mid-1] <= value:
                return mid
        if(a[mid]>value):
            return binary_search(a,start,mid -1, value)
        return binary_search(a,mid +1,end,value)
    return -1

n = 16 
comm = MPI.COMM_WORLD
size = comm.Get_size()
my_rank = comm.Get_rank()
tag = 0
master = 0
#a = [1,5,15,18,19,21,23,24,27,29,30,31,32,37,42,49]
#b = [2,3,4,13,15,19,20,22,28,29,38,41,42,43,48,49]
#a = [0, 1, 2, 4, 4, 5, 6, 9, 12, 14, 16, 18, 21, 21, 21, 22]
#b = [3, 6, 6, 8, 8, 11, 11, 12, 15, 15, 15, 16, 16, 16, 19, 22]
a = []
b = []
if my_rank == 0:
    for i in range(n):
        a_val = random.randint(0, 3)
        b_val = random.randint(0, 3)
        if i > 0:
            a_val = a_val + a[i-1]
            b_val = b_val + b[i-1]
        a.append(a_val)
        b.append(b_val)
    for source in range(1, size):
        comm.send(a, dest=source, tag=tag)
        comm.send(b, dest=source, tag=tag)
else:
    a = comm.recv(source=master, tag=tag)
    b = comm.recv(source=master, tag=tag)

k = int(math.log(n, 2))
a_start = (my_rank) * k
a_end = (my_rank) * k + (k-1)
b_start = 0
b_end = binary_search(b, 0, len(b) - 1, a[a_end])

if my_rank == 0:
    comm.send(b_end, dest=(my_rank+1), tag=tag)
elif my_rank == size - 1:
    b_start = comm.recv( source=(my_rank-1), tag=tag)
    b_end = n
else:
    b_start = comm.recv(source=(my_rank-1), tag=tag)
    comm.send(b_end, dest=(my_rank+1), tag=tag)

#print("rank " + str(my_rank) + ", a_start " + str(a_start) + ", a_end " + str(a_end) + ", b_start " + str(b_start) + ", b_end " + str(b_end))
b_end = b_end - 1
a_size = k
b_size = (b_end - b_start) + 1
c_size = a_size + b_size

a_new = a[a_start:a_end + 1]
a_new.append(math.inf)
if b_start > -1 and b_end > -1:
    b_new = b[b_start:b_end + 1]
else:
    b_new = []
b_new.append(math.inf)
c = []
i = 0
j = 0
for k in range(c_size):
    if a_new[i] <= b_new[j]:
        c.append(a_new[i])
        i = i + 1
    else:
        c.append(b_new[j])
        j = j + 1
'''
if my_rank == size - 1:
    print(a_new)
    print(b_new)
    print(c)
'''
if my_rank == 0:
    print(a)
    print(b)
    merge = []
    merge.extend(c)
    for source in range(1, size):
        c = comm.recv(source=source, tag=tag)
        #print(c)
        merge.extend(c)
    print(merge)
    print(len(merge))
else:
    comm.send(c, dest=master, tag=tag)