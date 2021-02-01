#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <gmp.h>
#include <mpi.h>

int main(int argc, char **argv)
{
    int my_rank;   //Rank of process
    int p;     //Number of process
    int source;
    int dest;
    int tag = 0;
    char message[100];
    MPI_Status status;
    //Start mpi
    MPI_Init(&argc, &argv);
    //Find process rank
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    //Find out number of processes
	MPI_Comm_size(MPI_COMM_WORLD, &p);
    if(my_rank != 0 )
    {
        //Create rank
        sprintf(message, "Greetings from process %d!",my_rank);
        dest = 0;
    }
    else
    {
        for (source = 1; source < p; source++)
        {
            MPI_Recv(message, 100, MPI_CHAR, source, tag, MPI_COMM_WORLD, &status);
            printf("%s\n",message);
        }
    }
    MPI_Finalize();
	return 0;
}