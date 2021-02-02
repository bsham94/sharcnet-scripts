#include <stdio.h>
#include <stdlib.h>
#include <gmp.h>
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
    int n = 1000000000000;
    int my_rank;        //Rank of process
    int processors;     //Number of process
    //int source;
    int tag = 0;
    int dest = 0;
    MPI_Status status;
    //Start mpi
    MPI_Init(&argc, &argv);
    //Find process rank
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    //Find out number of processes
    MPI_Comm_size(MPI_COMM_WORLD, &processors);

    if (my_rank == 0)
    {
        unsigned long int first, second, gap, largestFirst, largestSecond, largestGap = 0;
        for(int source = 1; source < processors; source++){
            MPI_Recv(&first, 1, MPI_UNSIGNED_LONG, source, tag, MPI_COMM_WORLD, &status);
            MPI_Recv(&second, 1, MPI_UNSIGNED_LONG, source, tag, MPI_COMM_WORLD, &status);
            MPI_Recv(&gap, 1, MPI_UNSIGNED_LONG, source, tag, MPI_COMM_WORLD, &status);
            if(gap > largestGap){
                largestFirst = first;
                largestSecond = second;
                largestGap = gap;
            }
        }
        printf("The largest gap is between %lu and %lu and it is %lu\n", largestFirst, largestSecond, largestGap);
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
        //printf("rank: %d, start: %lu, end: %lu\n", my_rank, start, end);

        mpz_t first, second, gap, largestFirst, largestSecond, largestGap, max;
        mpz_init_set_ui(first, start);
        mpz_init_set_ui(max, end);
        mpz_init(second);
        mpz_init(gap);
        mpz_init_set_ui(largestGap, 0);
        mpz_init(largestFirst);
        mpz_init(largestSecond);

        mpz_nextprime(second, first);
        if(mpz_probab_prime_p(first, 30) == 2) {
            mpz_sub(gap, second, first);
        }
        while(mpz_cmp(max, second) > 0){
            mpz_set(first, second);
            mpz_nextprime(second, first);
            mpz_sub(gap, second, first);
            if(mpz_cmp(gap, largestGap) > 0){
                mpz_set(largestFirst, first);
                mpz_set(largestSecond, second);
                mpz_set(largestGap, gap);
            }
        }

        unsigned long resultFirst = mpz_get_ui(largestFirst);
        MPI_Send(&resultFirst, 1, MPI_UNSIGNED_LONG, dest, tag, MPI_COMM_WORLD);
        unsigned long resultSecond = mpz_get_ui(largestSecond);
        MPI_Send(&resultSecond, 1, MPI_UNSIGNED_LONG, dest, tag, MPI_COMM_WORLD);
        unsigned long resultGap = mpz_get_ui(largestGap);
        MPI_Send(&resultGap, 1, MPI_UNSIGNED_LONG, dest, tag, MPI_COMM_WORLD);
        //printf("Rank: %d,", my_rank);
        //gmp_printf(" prime1 %Zd, prime2 %Zd, largest gap: %Zd\n", largestFirst, largestSecond, largestGap);
    }

    MPI_Finalize();
    return 0;
}