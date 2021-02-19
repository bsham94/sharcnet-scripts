#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>
#include <limits.h>
#include <time.h>

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
    //int a[16] = {1,5,15,18,19,21,23,24,27,29,30,31,32,37,42,49};
    //int b[16] = {2,3,4,13,15,19,20,22,28,29,38,41,42,43,48,49};
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
        int *a = (int*) malloc(sizeof(int)* n);
        int *b = (int*) malloc(sizeof(int)* n);
        if(myRank == masterProc)
        {
            // Create 2 pseudorandom arrays
            srand(time(NULL));
            printf("a: [ ");
            for(int i = 0; i < n; i++)
            {
                a[i] = rand() % 3;
                if (i > 0)
                    a[i] += a[i-1];
                printf("%d ", a[i]);
            }
            printf("]\n");
            printf("b: [ ");
            for(int i = 0; i < n; i++)
            {
                b[i] = rand() % 3;
                if (i > 0)
                    b[i] += b[i-1];
                printf("%d ", b[i]);
            }
            printf("]\n");
            for (int source = 1; source < processors; source++)
            {
                MPI_Send(a, n, MPI_INT, source, tag, MPI_COMM_WORLD);
                MPI_Send(b, n, MPI_INT, source, tag, MPI_COMM_WORLD);
            }
        }
        else
        {
            // Wait for arrays from master process
            MPI_Recv(a, n, MPI_INT, masterProc, tag, MPI_COMM_WORLD, &status);
            MPI_Recv(b, n, MPI_INT, masterProc, tag, MPI_COMM_WORLD, &status);
        }

        // Divide up array a for each process
        int aStart = calculateRange(myRank, n, k, r);
        int aEnd = calculateRange(myRank + 1, n, k, r) - 1;

        // Assume b starts at 0 until we recieve a message otherwise
        int bStart = 0;
        // Get the b array's end via binary search
        int bEnd = binarySearch(b, 0, n, a[aEnd]);

        // Send and receive b array start and end values
        if (myRank == masterProc) // The master process
        {
            // Master only sends
            MPI_Send(&bEnd, 1, MPI_INT, myRank + 1, tag, MPI_COMM_WORLD);
        }
        else if (myRank == processors - 1) // Last process
        {
            // Final process only recieves
            MPI_Recv(&bStart, 1, MPI_INT, myRank - 1, tag, MPI_COMM_WORLD, &status);
            bEnd++;
        }
        else// Slave process
        {
            // All other processes send and recieve
            MPI_Send(&bEnd, 1, MPI_INT, myRank + 1, tag, MPI_COMM_WORLD);
            MPI_Recv(&bStart, 1, MPI_INT, myRank - 1, tag, MPI_COMM_WORLD, &status);
        }

        // Calculate the size that array c should be
        int sizeA = k;
        int sizeB = (bEnd - bStart);
        int sizeC = sizeA + sizeB;
        // Dynamically create array c
        int *c = malloc(sizeof(int)* sizeC);

        /*
         * Append INT_MAX to the end of a and b.
         * This will help us later when sorting the smallest values.
         * It prevents us from going out of bounds.
         */
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

        if(myRank == processors - 1){
            printf("last a: [ ");
            for(int i = 0; i < sizeA; i++)
            {
                printf("%d ", aNew[i]);
            }
            printf("]\n");
            printf("bstart: %d, bend: %d\n", bStart, bEnd);
            printf("last b: [ ");
            for(int i = 0; i < sizeB; i++)
            {
                printf("%d ", bNew[i]);
            }
            printf("]\n");
        }
        
        // Merge a and b
        int i = 0;
        int j = 0;
        for(int k = 0; k < sizeC; k++)
        {
            /* 
             * If a is smaller, append it to c and iterate.
             * Since we added INT_MAX, it will stop iterating
             * once it reaches that.
             */
            if(aNew[i] <= bNew[j]){
                c[k] = aNew[i];
                i++;
            }
            else // b is smaller
            {
                c[k] = bNew[j];
                j++;
            }
        }

        if (myRank == masterProc){ // The master process
            // Merge array will store all our combined arrays.
            int *merge = malloc(sizeof(int)* (n*2) + 1);
            int mergeLoc = 0;

            for(int i = 0; i < sizeC; i++)
            {
                // Add to merge array
                merge[mergeLoc] = c[i];
                mergeLoc++;
            }

            // Wait for c arrays from each processes
            for (int source = 1; source < processors; source++)
            {
                // Receive from slave processes
                //MPI_Recv(&sizeC, 1, MPI_INT, source, tag, MPI_COMM_WORLD, &status);
                MPI_Probe(source, tag, MPI_COMM_WORLD, &status);
                MPI_Get_count(&status, MPI_INT, &sizeC);
                //int *cNew = malloc(sizeof(int)* sizeC);
                c = realloc(c, sizeof(int) * sizeC);
                MPI_Recv(c, sizeC, MPI_INT, source, tag, MPI_COMM_WORLD, &status);
                for(int i = 0; i < sizeC; i++)
                {
                    // Add to merge array
                    merge[mergeLoc] = c[i];
                    mergeLoc++;
                }
            }

            // Output
            printf("merge: [ ");
            for (int i = 0; i < mergeLoc; i++){
                printf("%d ", merge[i]);
            }
            printf("]\n");

            free(merge);
        }
        else // Slave processes
        {
            // Send c array to master process to merge
            //MPI_Send(&sizeC, 1, MPI_INT, masterProc, tag, MPI_COMM_WORLD);
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