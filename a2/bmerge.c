#include <stdio.h>
#include <stdlib.h>
#include <gmp.h>
#include <mpi.h>
#include <math.h>
#include <string.h>

#pragma warning(disable:4996)

 void merge(int **c, int *a, int *b, int k,int a_start,int a_end, int start,int end)
{
    int i = 0;
    int j = 0;
    int sizeOfB = end - start;
    int *new_a = malloc(sizeof(int)*(k+1));
    new_a[k] = INT_MAX;
    int *new_b = (int *) malloc(sizeof(int)*(sizeOfB+1));
    new_b[sizeOfB] = INT_MAX;
    *c = (int*) malloc(sizeof(int)*((sizeOfB)+k));
    
    //Copy array into an array with infinity appended to the end
    //Can probably replace with memcpy
    for (int t = a_start; t <= a_end; t++)
    {
        new_a[t-a_start] = a[t];
    }
    for (int s = 0; s <(sizeOfB)  ;s++)
    {
        new_b[s] = b[start+s];
    }
    for(int q = 0; q <(sizeOfB)+k ;q++)
    {
        if(new_a[i] <= new_b[j])
        {
            (*c)[q] = new_a[i];
            i++;
        }
        else
        {
            (*c)[q] = new_b[j];
            j++;
        }
        //printf("%d, ",(*c)[q]);
    }
    //printf("\n");
    free(new_a);
    free(new_b);
}

// l is the start, r is the size, x is the value
int binarySearch(int arr[], int l, int r, int x) 
{ 
    if (r >= l) { 
        int mid = l + (r - l) / 2; 
        if (arr[mid] >= x)
        {           
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


int main(int argc, char** argv)
{
    int n = 16;
    int my_rank;        //Rank of process
    int processors;     //Number of process
    //int source;
    int tag = 0;
    MPI_Status status;
    //Start mpi
    MPI_Init(&argc, &argv);
    //Find process rank
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    //Find out number of processes
    MPI_Comm_size(MPI_COMM_WORLD, &processors);
    int a[16] = {1,5,15,18,19,21,23,24,27,29,30,31,32,37,42,49};
    int b[16] = {2,3,4,13,15,19,20,22,28,29,38,41,42,43,48,49};
    //int c[16] = {};
    int k = log2(n);
    int a_start = (my_rank)*k;
    int a_end = (my_rank)*k + (k-1);
    int b_end = binarySearch(b,0,n,a[a_end]);
    int b_start = 0;
    int *c = NULL;
    //printf("Rank: %d  Start Value: %d  End Value: %d Start: %d  End %d  T: %d  J: %d\n",my_rank, a[a_start],a[a_end],a_start, a_end, b_end, b[b_end]);

    if(my_rank == 0)
    {
        MPI_Send(&b_end, 1, MPI_INT, my_rank + 1, tag, MPI_COMM_WORLD);
        //printf("Rank %d  Start: %d  End: %d\n",my_rank,b_start,b_end-1);
        //printf("Rank: %d  A Start Value: %d  A End Value: %d A Start: %d  A End %d  B Start: %d B End: %d  J: %d\n",my_rank, a[a_start],a[a_end],a_start, a_end, b_start, b_end-1, b[b_end]);
        merge(&c,a,b,k,a_start,a_end,b_start,b_end);
    }
    else if((my_rank)*k + (k) == n)
    {     
        MPI_Recv(&b_start, 1, MPI_INT, my_rank - 1, tag, MPI_COMM_WORLD,&status);
        //printf("Rank: %d  A Start Value: %d  A End Value: %d A Start: %d  A End %d  B Start: %d B End: %d  J: %d\n",my_rank, a[a_start],a[a_end],a_start, a_end, b_start, b_end, b[b_end]);
        merge(&c,a,b,k,a_start,a_end,b_start,b_end+1);
    }
    else
    {
        MPI_Send(&b_end, 1, MPI_INT, my_rank + 1, tag, MPI_COMM_WORLD);
        MPI_Recv(&b_start, 1, MPI_INT, my_rank - 1, tag, MPI_COMM_WORLD,&status);
        //printf("Rank: %d  A Start Value: %d  A End Value: %d A Start: %d  A End %d  B Start: %d B End: %d  J: %d\n",my_rank, a[a_start],a[a_end],a_start, a_end, b_start, b_end-1, b[b_end]);
        merge(&c,a,b,k,a_start,a_end,b_start,b_end);
    }       
    if(my_rank == 0)
    {
        if(c == NULL)
        {
            printf("NULL\n");            
        }
        else
        {          
            for (int i = 0; i < (b_end-b_start)+k; i++)
            {
                printf("%d,",c[i]);
            }
            printf("\n");          
        }
    }
    free(c);
    MPI_Finalize();
    return 0;
}