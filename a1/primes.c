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


unsigned long long mpz_get_ull(mpz_t op){
    // Referenced from https://stackoverflow.com/a/6248913
    // Since gmplib lacks proper mpz_get_ull function
    unsigned long long rop = 0;
    mpz_export( // Converts a word of binary data to an mpz_t
        &rop,   // (Rop) ull to return
        0,      // (Count) Number of binary words produced (not needed)
        -1,     // (Order) Least significant word first
        sizeof(unsigned long long),
        0,      // (Endian) Native endianness of host CPU
        0,      // (Nails) Produce full words
        op      // (Op) mpz_t to convert
    );
    return rop;
}

void mpz_init_set_ull(mpz_t rop, unsigned long long op){
    // Referenced from https://stackoverflow.com/a/6248913
    // Since gmplib lacks proper mpz_init_set_ull function
    mpz_init(rop);
    mpz_import(
        rop,    // (Rop) mpz_t to return
        1,      // (Count) 1 binary word
        -1,     // (Order) Least significant word first
        sizeof(unsigned long long),
        0,      // (Endian) Native endianness of host CPU
        0,      // (Nails) Use full words
        &op     // (Op) ull to convert
    );
}

int main(int argc, char** argv)
{
    /*
     * Using 'unsigned long long' since 'unsigned long' has max
     * 4,294,967,295
     * We need to reach at least 1 trillion.
     * 
     * 'unsigned long long' has max
     * 18,446,744,073,709,551,615
     * which is plenty (more than enough).
    */
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

    if (my_rank == 0)   // The master process
    {
        // Get starting time
        double startTime = MPI_Wtime();


        // Wait for responses from all processors
        unsigned long long first, second, gap, largestFirst, largestSecond, largestGap = 0;
        for(int source = 1; source < processors; source++){
            // Get 3 messages from each processor
            MPI_Recv(&first, 1, MPI_UNSIGNED_LONG_LONG, source, tag, MPI_COMM_WORLD, &status);
            MPI_Recv(&second, 1, MPI_UNSIGNED_LONG_LONG, source, tag, MPI_COMM_WORLD, &status);
            MPI_Recv(&gap, 1, MPI_UNSIGNED_LONG_LONG, source, tag, MPI_COMM_WORLD, &status);

            // Check if this response was the biggest one
            if(gap > largestGap){
                // If so, store it
                largestFirst = first;
                largestSecond = second;
                largestGap = gap;
            }
        }

        // Output the results
        printf("The largest gap is between %llu and %llu and it is %llu.\n", largestFirst, largestSecond, largestGap);
        double endTime = (MPI_Wtime() - startTime);
        printf("Completed in %.2lf seconds on %d processors.\n", endTime, processors - 1);
    }
    else // Child process
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
        mpz_init_set_ull(first, start);
        mpz_init_set_ull(max, end);
        mpz_init(second);
        mpz_init(gap);
        mpz_init_set_ull(largestGap, 0);
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
       unsigned long long resultFirst = mpz_get_ull(largestFirst);
       MPI_Send(&resultFirst, 1, MPI_UNSIGNED_LONG_LONG, dest, tag, MPI_COMM_WORLD);
       unsigned long long resultSecond = mpz_get_ull(largestSecond);
       mpz_export(&resultSecond, 0, -1, sizeof(unsigned long long), 0, 0, largestSecond);
       MPI_Send(&resultSecond, 1, MPI_UNSIGNED_LONG_LONG, dest, tag, MPI_COMM_WORLD);
       unsigned long long resultGap = mpz_get_ull(largestGap);
       mpz_export(&resultGap, 0, -1, sizeof(unsigned long long), 0, 0, largestGap);
       MPI_Send(&resultGap, 1, MPI_UNSIGNED_LONG_LONG, dest, tag, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return 0;
}