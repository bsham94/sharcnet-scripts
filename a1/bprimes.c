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
    int source;
    int dest;
    int tag = 0;
    char message[100];
    MPI_Status status;
    unsigned long long *ranges;
    //Start mpi
    MPI_Init(&argc, &argv);
    //Find process rank
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    //Find out number of processes
	MPI_Comm_size(MPI_COMM_WORLD, &processors);
    
    //int k = 63;
    //processors = 6;
    ranges = (unsigned long long *) malloc(sizeof(unsigned long long)*processors);
    int result = (n / processors);

    for (int i = 0; i < processors; i++)
    {
        int number = 0;
        if (i < (n % processors))
        {
            number = i;
        }
        else 
        { 
            number = n % processors;
        }
        ranges[i] = (i * result) + number;
        printf("%llu\n", ranges[i]);
    }

    


    if(my_rank != 0 )
    {
        //Create rank
        sprintf(message, "Greetings from process %d!",my_rank);
        dest = 0;
        MPI_Send(message, strlen(message)+1, MPI_CHAR, dest, tag, MPI_COMM_WORLD);
    }
    else
    {
        for (source = 1; source < processors; source++)
        {
            MPI_Recv(message, 100, MPI_CHAR, source, tag, MPI_COMM_WORLD, &status);
            printf("%s\n",message);
        }
    }
    free(ranges);
    MPI_Finalize();
	return 0;
}