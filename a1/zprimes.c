#include <stdio.h>
#include "mpi.h"
#include "gmp.h"
#include <string.h>

int main(int argc, char** argv) {
    int myRank; // this rank
    int numProcesses; // number of processes running
    int source; // rank of the sender
    int dest; // rank of the receiver
    int tag = 0;
    char message[100];
    unsigned int max = 1000000000;
    MPI_Status status; // return status for receive message

    // Start MPI
    if (MPI_Init(&argc, &argv) != MPI_SUCCESS) {
        printf("[ ERROR ]");
    }

    // Get process rank
    MPI_Comm_rank(MPI_COMM_WORLD, &myRank);

    // Get number of processes
    MPI_Comm_size(MPI_COMM_WORLD, &numProcesses);

    // Get range
    unsigned int start[8];
    unsigned int prev = 0;
    for(int i = 1; i < max; i++){
        start[i] = prev + (max / numProcesses);
        if(i < (max % numProcesses)){
            // add 1
            start[i]++;
        }
        prev = start[i];
    }

    if (myRank == 0) {
        // Main process
        for (source = 1; source < numProcesses; source++){
            MPI_Recv(message, 100, MPI_CHAR, source, tag, MPI_COMM_WORLD, &status);
            printf("%s\n",message);
        }
    } else {
        sprintf(message, "My starting number is %d!", start[myRank]);
        dest = 0;
        MPI_Send(message, strlen(message)+1, MPI_CHAR, dest, tag, MPI_COMM_WORLD);
    }

    MPI_Finalize();
}