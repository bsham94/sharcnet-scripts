#include <stdio.h>
#include <stdlib.h>
#include <gmp.h>
#include <mpi.h>

unsigned long long calculateRange(int rank, unsigned long long n, int p)
{
    /* 
     * Gets the data parallelism range.
     * ==> istart,p=p[n/P]+min(p,mod(n,P))
     * 
     * Essentially, pigeonhole principal
     * n => pigeons
     * p => pigeonholes
     * 
     * The large number 'n' is divided up so that each processor gets a section of
     * the number to search for primes within.
    */
    unsigned long long result = (n / p);
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

unsigned long long mpz_get_ull(mpz_t op)
{
    /* 
     * Referenced from https://stackoverflow.com/a/6248913
     * Since gmplib lacks proper mpz_get_ull function
     * This will convert an unsigned long long to an mpz_t
     */
    unsigned long long rop = 0;
    mpz_export( // Converts a word of binary data to an mpz_t
        &rop,   // (Rop) ull to return
        0,      // (Count) Number of binary words produced (not needed)
        -1,     // (Order) Least significant word first
        sizeof(unsigned long long),
        0, // (Endian) Native endianness of host CPU
        0, // (Nails) Produce full words
        op // (Op) mpz_t to convert
    );
    return rop;
}

void mpz_init_set_ull(mpz_t rop, unsigned long long op)
{
    /*
     * Referenced from https://stackoverflow.com/a/6248913
     * Since gmplib lacks proper mpz_init_set_ull function
     * This will convert an mpz_t to an unsigned long long
     */
    mpz_init(rop);
    mpz_import(
        rop, // (Rop) mpz_t to return
        1,   // (Count) 1 binary word
        -1,  // (Order) Least significant word first
        sizeof(unsigned long long),
        0,  // (Endian) Native endianness of host CPU
        0,  // (Nails) Use full words
        &op // (Op) ull to convert
    );
}

int main(int argc, char **argv)
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
    unsigned long long n = 1000000000; // Number to count to
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

    // Get starting time
    double startTime = MPI_Wtime();

    // Calculate start of current processes searching range
    unsigned long long start = calculateRange(myRank, n, processors);

    // Calculate end
    unsigned long long end = calculateRange(myRank + 1, n, processors) - 1;
    if (myRank == (processors - 1))
    {
        // Final processor reaches to the end
        end++;
    }

    // Initialize all the mpz_ts
    mpz_t first, second, gap, largestFirst, largestSecond, largestGap, max;
    mpz_init_set_ull(first, start);
    mpz_init_set_ull(max, end);
    mpz_init(second);
    mpz_init(gap);
    mpz_init_set_ull(largestGap, 0); // Set to 0 because we'll be comparing it later
    mpz_init(largestFirst);
    mpz_init(largestSecond);

    // Determine if we're starting with a prime
    mpz_nextprime(second, first);
    if (mpz_probab_prime_p(first, 30) == 2)
    {
        mpz_sub(gap, second, first);
    }

    // Loop until we reach the max, checking for the largest prime gap
    while (mpz_cmp(max, second) > 0)
    {
        mpz_set(first, second);
        mpz_nextprime(second, first);
        mpz_sub(gap, second, first);
        if (mpz_cmp(gap, largestGap) > 0)
        {
            // Store the largest primes and their gap
            mpz_set(largestFirst, first);
            mpz_set(largestSecond, second);
            mpz_set(largestGap, gap);
        }
    }

    // Export back to unsigned long long
    unsigned long long resultFirst = mpz_get_ull(largestFirst);
    unsigned long long resultSecond = mpz_get_ull(largestSecond);
    unsigned long long resultGap = mpz_get_ull(largestGap);

    if (myRank == masterProc) // The master process
    {
        // Master process has largest for now
        unsigned long long largestFirst = resultFirst;
        unsigned long long largestSecond = resultSecond;
        unsigned long long largestGap = resultGap;

        /*
         * Wait for responses from all processors.
         * The processors will each return their largest prime gap they found.
         * This master process will determine which was the largest found and
         * output the result
         */
        for (int source = 1; source < processors; source++)
        {
            // Get 3 messages from each processor
            MPI_Recv(&resultFirst, 1, MPI_UNSIGNED_LONG_LONG, source, tag, MPI_COMM_WORLD, &status);
            MPI_Recv(&resultSecond, 1, MPI_UNSIGNED_LONG_LONG, source, tag, MPI_COMM_WORLD, &status);
            MPI_Recv(&resultGap, 1, MPI_UNSIGNED_LONG_LONG, source, tag, MPI_COMM_WORLD, &status);

            // Check if this response was the biggest one
            if (resultGap > largestGap)
            {
                // If so, store it
                largestFirst = resultFirst;
                largestSecond = resultSecond;
                largestGap = resultGap;
            }
        }

        // Stop & calculate the running time
        // For parallel benchmarking
        double endTime = (MPI_Wtime() - startTime);

        /* 
         * Output the results
         * We output processors - 1 because processor rank=0 didn't help in the calculation
         * Eg. If we want to test 8 processors, we use ntasks=9 because one of them will sit
         *  and wait for messages
         */
        printf("The largest gap is between %llu and %llu and it is %llu.\n", largestFirst, largestSecond, largestGap);
        printf("Completed in %.2lf seconds on %d processors.\n", endTime, processors);
    }
    else // Slave process
    {
        // Send all three to rank=0
        MPI_Send(&resultFirst, 1, MPI_UNSIGNED_LONG_LONG, masterProc, tag, MPI_COMM_WORLD);
        mpz_export(&resultSecond, 0, -1, sizeof(unsigned long long), 0, 0, largestSecond);
        MPI_Send(&resultSecond, 1, MPI_UNSIGNED_LONG_LONG, masterProc, tag, MPI_COMM_WORLD);
        mpz_export(&resultGap, 0, -1, sizeof(unsigned long long), 0, 0, largestGap);
        MPI_Send(&resultGap, 1, MPI_UNSIGNED_LONG_LONG, masterProc, tag, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return 0;
}