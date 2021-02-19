#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>
#include <limits.h>

int calculateRange(int rank, int n, int k, int r)
{
    // Pigeonhole principle
    int number = 0;
    // min(rank, mod(n,p)) is added to range
    if (rank < (n % r))
    {
        number = rank;
    }
    else
    {
        number = n % r;
    }
    return (rank * k) + number;
}

int binarySearch(int arr[], int l, int r, int x) 
{ 
    if (r >= l) { 
        int mid = l + (r - l) / 2; 
        if (arr[mid] >= x){
            
            if((mid-1) >= 0 &&  arr[mid-1] < x)
            {
                return mid; 
            }
            
        }            
        if (arr[mid] > x) 
        {
            return binarySearch(arr, l, mid - 1, x); 
        }
        return binarySearch(arr, mid + 1, r, x); 
    } 
    return -1; 
} 

int main(int argc, char **argv)
{
    int n = 16;
    int a[16] = {1,5,15,18,19,21,23,24,27,29,30,31,32,37,42,49};
    int b[16] = {2,3,4,13,15,19,20,22,28,29,38,41,42,43,48,49};
    int myRank;                           // Rank of process
    int processors;                       // Number of process
    int tag = 0;                          // Message tag
    int masterProc = 0;                   // Destination to send (all send to rank=0)
    MPI_Status status;                    // Status
    //Start mpi
    MPI_Init(&argc, &argv);
    //Find process rank
    MPI_Comm_rank(MPI_COMM_WORLD, &myRank);
    //Find out number of processes
    MPI_Comm_size(MPI_COMM_WORLD, &processors);

    int k = log2(n);
    int r = n/k;
    if (r <= processors) { // Check if we have enough processors
        int aStart = calculateRange(myRank, n, k, r);
        int aEnd = calculateRange(myRank + 1, n, k, r) - 1;

        int bStart = 0;
        int bEnd = binarySearch(b, 0, n, a[aEnd]);

        // Send and receive 
        if (myRank == masterProc) // The master process
        {
            MPI_Send(&bEnd, 1, MPI_INT, myRank + 1, tag, MPI_COMM_WORLD);
        }
        else if (myRank == processors - 1) // Last process
        {
            MPI_Recv(&bStart, 1, MPI_INT, myRank - 1, tag, MPI_COMM_WORLD, &status);
        }
        else// Slave process
        {
            MPI_Send(&bEnd, 1, MPI_INT, myRank + 1, tag, MPI_COMM_WORLD);
            MPI_Recv(&bStart, 1, MPI_INT, myRank - 1, tag, MPI_COMM_WORLD, &status);
        }

        //printf("rank:%d, min: %d, max: %d, diff: %d\n", myRank, bStart, bEnd, (bEnd - bStart));

        int sizeA = k;
        int sizeB = (bEnd - bStart);
        int sizeC = sizeA + sizeB;
        int *c = malloc(sizeof(int)* sizeC);

        int *aNew = malloc(sizeof(int) * (k));
        aNew[k] = INT_MAX;
        for(int i = aStart; i <= aEnd; i++)
        {
            aNew[i - aStart] = a[i];
        }

        int *bNew = malloc(sizeof(int) * (sizeB + 1));
        bNew[sizeB] = INT_MAX;
        for(int i = 0; i < sizeB; i++)
        {
            bNew[i] = b[bStart + i];
        }

        int i = 0;
        int j = 0;
        for(int k = 0; k < sizeC; k++)
        {
            if(aNew[i] <= bNew[j]){
                c[k] = aNew[i];
                i++;
            }
            else
            {
                c[k] = bNew[j];
                j++;
            }
        }

        if (myRank == masterProc){ // The master process
            /* 
             * Merge array will store all our combined arrays.
             * It's size is (the size of c) * (k amount of processors),
             * since each k processor created a c array.
             */
            int *merge = malloc(sizeof(int)* (sizeC * k));
            for(int i = 0; i < sizeC; i++)
            {
                merge[myRank + i] = c[i];
            }
            
            // Wait for merged arrays from each processes
            for (int source = 1; source < processors; source++)
            {
                //printf("recieving from %d\n", source);
                MPI_Recv(c, sizeC, MPI_INT, source, tag, MPI_COMM_WORLD, &status);
                for(int i = 0; i < sizeC; i++)
                {
                    //printf("source rank: %d, %d\n",source, (source * sizeC) + i);
                    merge[(source*sizeC) + i] = c[i];
                }
            }

            // Merge all received arrays
            printf("[ ");
            for (int i = 0; i < (sizeC * k); i++){
                printf("%d ", merge[i]);
            }
            printf("]\n");
            free(merge);
        }
        else // Slave processes
        {
            MPI_Send(c, sizeC, MPI_INT, masterProc, tag, MPI_COMM_WORLD);
        }

        free(aNew);
        free(bNew);
        free(c);
    }
    else // Not enough processors
    {   
        // Only print in master so we only get 1 output
        if (myRank == masterProc) // The master process
        {
            printf("Not enough processors allocated.\n");
        }
    }
    

    MPI_Finalize();
    return 0;
}