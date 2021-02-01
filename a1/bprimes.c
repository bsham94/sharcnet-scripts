#include <stdio.h>
#include <stdlib.h>
//#include <gmp.h>
#include <mpi.h>
#include <math.h>
#include <string.h>
#pragma warning(disable:4996)
int main(int argc, char **argv)
{
    int n = 63;
    int my_rank;   //Rank of process
    int processors;     //Number of process
    //int source;
    int tag = 0;
    MPI_Status status;
    unsigned long int *ranges;
    //Start mpi
    MPI_Init(&argc, &argv);
    //Find process rank
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    //Find out number of processes
	MPI_Comm_size(MPI_COMM_WORLD, &processors);
    //processors = 7;
    if(my_rank == 0 )
    {   
        int result = (n / (processors-1));
        ranges = (unsigned long int*) malloc(sizeof(long long) * (processors - 1));
        for (int i = 0; i < (processors-1); i++)
        {
            int number = 0;
            if (i < (n % (processors-1)))
            {
                number = i;
            }
            else
            {
                number = n % (processors-1);
            }
            ranges[i] = (i * result) + number;
        }
        for (int i = 1; i < processors; i++)
        {
            if (i == (processors-1))
            {
                MPI_Send(&(ranges[i - 1]), 1, MPI_UNSIGNED_LONG, i, tag, MPI_COMM_WORLD);
            }
            else
            {
                MPI_Send(&(ranges[i - 1]), 1, MPI_UNSIGNED_LONG, i, tag, MPI_COMM_WORLD);
                MPI_Send(&(ranges[i]), 1, MPI_UNSIGNED_LONG, i, tag, MPI_COMM_WORLD);

            }                        
        }
        free(ranges);
    }
    else if (my_rank == (processors -1))
    {
        unsigned long int start = 0;
        MPI_Recv(&start, 1, MPI_UNSIGNED_LONG, 0, tag, MPI_COMM_WORLD, &status);
        printf("Rank: %d\n", my_rank);
        printf("%lu\n", start);
        printf("%lu\n", n);
    }
    else
    {   
        unsigned long int start = 0;
        unsigned long int end = 0;
        MPI_Recv(&start, 1, MPI_UNSIGNED_LONG, 0, tag, MPI_COMM_WORLD, &status);
        MPI_Recv(&end, 1, MPI_UNSIGNED_LONG, 0, tag, MPI_COMM_WORLD, &status);
        printf("Rank: %d\n", my_rank);
        printf("%lu\n",start);
        printf("%lu\n", end-1);
    }
    
    MPI_Finalize();
    return 0;
}