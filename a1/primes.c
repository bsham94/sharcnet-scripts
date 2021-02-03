#include <stdio.h>
#include <stdlib.h>
#include <gmp.h>
#include <mpi.h>

unsigned long long calculateRange(int rank, unsigned long long n, int p) {
    /* Essentially, pigeonhole principal
     * n => pigeons
     * p => pigeonholes
    */
    int result = (n / p);
    int number = 0;
    // min(rank, mod(n,p)) is added to range
    if (rank < (n % p))
    {
        number = rank;
    }
    else
    {
        number = n % p;
    }
    return (rank * result) + number;
}

int main(int argc, char** argv)
{
    unsigned long long n = 1000000000000;    // Number to count to
    int my_rank;        // Rank of process
    int processors;     // Number of process
    int tag = 0;        // Message tag
    int dest = 0;       // Destination to send (all send to rank=0)
    MPI_Status status;  // Status

    //Start mpi
    MPI_Init(&argc, &argv);
    //Find process rank
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    //Find out number of processes
    MPI_Comm_size(MPI_COMM_WORLD, &processors);

    if (my_rank == 0)
    {
        // Get starting time
        double startTime = MPI_Wtime();


        // Wait for responses from all processors
        unsigned long long first, second, gap, largestFirst, largestSecond, largestGap = 0;
        for(int source = 1; source < processors; source++){
            MPI_Recv(&first, 1, MPI_UNSIGNED_LONG, source, tag, MPI_COMM_WORLD, &status);
            MPI_Recv(&second, 1, MPI_UNSIGNED_LONG, source, tag, MPI_COMM_WORLD, &status);
            MPI_Recv(&gap, 1, MPI_UNSIGNED_LONG, source, tag, MPI_COMM_WORLD, &status);
            // Check if this response was the biggest one
            if(gap > largestGap){
                largestFirst = first;
                largestSecond = second;
                largestGap = gap;
            }
        }
        printf("The largest gap is between %llu and %llu and it is %llu\n", largestFirst, largestSecond, largestGap);
        double endTime = (MPI_Wtime() - startTime);
        printf("Completed in %.2lf seconds on %d processors.\n", endTime, processors - 1);
    }
    else
    {
        // Calculate start
        unsigned long long start = calculateRange(my_rank - 1, n, processors - 1);

        // Calculate end
        unsigned long long end = calculateRange(my_rank, n, processors - 1) - 1;
        if (my_rank == (processors - 1)) {
            // Final processor reaches to the end
            end++;
        }

        // Initialize all the mpz_ts
        mpz_t first, second, gap, largestFirst, largestSecond, largestGap, max;
        //mpz_init_set_ui(first, start);
        mpz_init(first);
        mpz_import(first, 1, -1, sizeof start, 0, 0, &start);

        //mpz_init_set_ui(max, end);
        mpz_init(max);
        mpz_import(max, 1, -1, sizeof end, 0, 0, &end);

        mpz_init(second);
        mpz_init(gap);

        //mpz_init_set_ui(largestGap, 0);
        unsigned long long currentGap = 0;
        mpz_init(largestGap);
        mpz_import(largestGap, 1, -1, sizeof currentGap, 0, 0, &currentGap);

        mpz_init(largestFirst);
        mpz_init(largestSecond);

        // Determine if we're starting with a prime
        mpz_nextprime(second, first);
        if(mpz_probab_prime_p(first, 30) == 2) {
            mpz_sub(gap, second, first);
        }

        // Loop until we reach the max, checking for the largest prime gap
        while(mpz_cmp(max, second) > 0){
            mpz_set(first, second);
            mpz_nextprime(second, first);
            mpz_sub(gap, second, first);
            if(mpz_cmp(gap, largestGap) > 0){
                // Store the largest primes and their gap
                mpz_set(largestFirst, first);
                mpz_set(largestSecond, second);
                mpz_set(largestGap, gap);
            }
        }

        // Send all three to rank=0
        /*
        unsigned long long resultFirst = mpz_get_ui(largestFirst);
        MPI_Send(&resultFirst, 1, MPI_UNSIGNED_LONG, dest, tag, MPI_COMM_WORLD);
        unsigned long long resultSecond = mpz_get_ui(largestSecond);
        MPI_Send(&resultSecond, 1, MPI_UNSIGNED_LONG, dest, tag, MPI_COMM_WORLD);
        unsigned long long resultGap = mpz_get_ui(largestGap);
        MPI_Send(&resultGap, 1, MPI_UNSIGNED_LONG, dest, tag, MPI_COMM_WORLD);
        */
       unsigned long long resultFirst = 0;
       mpz_export(&resultFirst, 0, -1, sizeof(unsigned long long), 0, 0, largestFirst);
       MPI_Send(&resultFirst, 1, MPI_UNSIGNED_LONG_LONG, dest, tag, MPI_COMM_WORLD);
       unsigned long long resultSecond = 0;
       mpz_export(&resultSecond, 0, -1, sizeof(unsigned long long), 0, 0, largestSecond);
       MPI_Send(&resultSecond, 1, MPI_UNSIGNED_LONG_LONG, dest, tag, MPI_COMM_WORLD);
       unsigned long long resultGap = 0;
       mpz_export(&resultGap, 0, -1, sizeof(unsigned long long), 0, 0, largestGap);
       MPI_Send(&resultGap, 1, MPI_UNSIGNED_LONG_LONG, dest, tag, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return 0;
}