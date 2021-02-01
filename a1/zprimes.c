#include <stdio.h>
#include <stdlib.h>
//#include <gmp.h>
#include <mpi.h>
#include <math.h>
#include <string.h>
#pragma warning(disable:4996)

int calculateRange(int rank, int n, int processors) {
    int result = (n / processors);
    int number = 0;
    if (rank < (n % processors))
    {
        number = rank;
    }
    else
    {
        number = n % processors;
    }
    return (rank * result) + number;
}

int main(int argc, char** argv)
{
    int n = 63;
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

    if (my_rank == 0)
    {
    }
    else
    {
        // Calculate start
        unsigned long int start = calculateRange(my_rank - 1, n, processors - 1);

        // Calculate end
        unsigned long int end = calculateRange(my_rank, n, processors - 1) - 1;
        if (my_rank == (processors - 1)) {
            // Final processor
            end++;
        }
        printf("rank: %d, start: %lu, end: %lu\n", my_rank, start, end);
    }

    MPI_Finalize();
    return 0;
}