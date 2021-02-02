#include <stdio.h>
#include <stdlib.h>
#include <gmp.h>
#include <mpi.h>
#include <math.h>
#include <string.h>

//#pragma warning(disable:4996)

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
void calculatePrimes(unsigned long int end, mpz_t finalPrimeOne, mpz_t finalPrimeTwo, mpz_t largestGap)
{
    mpz_t  firstPrime, secondPrime, thirdPrime,endNumber;   
    mpz_init(firstPrime);
    mpz_init(secondPrime);
    mpz_init(thirdPrime);
    mpz_init(endNumber);
    mpz_set_ui(endNumber,end);
    mpz_nextprime(firstPrime,finalPrimeOne);
    mpz_nextprime(secondPrime,firstPrime);
    mpz_set(finalPrimeTwo,secondPrime);
    mpz_sub(largestGap,secondPrime,firstPrime);
    while (mpz_cmp(thirdPrime,endNumber) < 0)
    {
        mpz_nextprime(thirdPrime,secondPrime);
        if(mpz_cmp(thirdPrime,endNumber) < 0)
        {
            mpz_t newGap;
            mpz_init(newGap);
            mpz_sub(newGap,thirdPrime,secondPrime);
            if(mpz_cmp(newGap,largestGap) > 0)
            {
                mpz_set(largestGap,newGap);
                mpz_set(finalPrimeOne,secondPrime);
                mpz_set(finalPrimeTwo,thirdPrime);                
            }
            mpz_set(firstPrime,secondPrime);
            mpz_set(secondPrime,thirdPrime);
        }
    }       
    
}
int main(int argc, char** argv)
{
    int n = 1000;
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
        unsigned long int first, second, gap, firstLargest, secondLargest, largestGap = 0;
        //Send range of numbers to all processes
        for (int i = 1; i < processors; i++)
        {           
            //Sends start and end range to all process but the last one
            //Last one stops at n.
            MPI_Recv(&first, 1, MPI_UNSIGNED_LONG, i, tag, MPI_COMM_WORLD,&status);
            MPI_Recv(&second, 1, MPI_UNSIGNED_LONG, i, tag, MPI_COMM_WORLD,&status);
            MPI_Recv(&gap, 1, MPI_UNSIGNED_LONG, i, tag, MPI_COMM_WORLD,&status);
            //printf("Largest gap is between %lu and %lu and it is %lu\n", first,second,largestGap);
            
            if(gap > largestGap)
            {
                firstLargest = first;
                secondLargest = second;
                largestGap = gap;
            }            
        }
        printf("Largest gap is between %lu and %lu and it is %lu\n", firstLargest,secondLargest,largestGap);
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
        mpz_t finalPrimeOne,finalPrimeTwo,largestGap;       
        mpz_init(finalPrimeOne);
        mpz_init(finalPrimeTwo);
        mpz_init(largestGap);
        mpz_set_ui(finalPrimeOne, start);        
        //gmp_printf("%Zd\n",finalPrimeOne);
        calculatePrimes(end,finalPrimeOne,finalPrimeTwo,largestGap);
        //gmp_printf("rank: %d, start: %lu, end: %lu Prime 1: %Zd  Prime 2: %Zd  Largest Gap: %Zd \n", my_rank, start, end, finalPrimeOne,finalPrimeTwo,largestGap);
        unsigned long firstResult = mpz_get_ui(finalPrimeOne);
        MPI_Send(&firstResult, 1, MPI_UNSIGNED_LONG, 0, tag, MPI_COMM_WORLD);
        unsigned long secondResult = mpz_get_ui(finalPrimeTwo);
        MPI_Send(&secondResult, 1, MPI_UNSIGNED_LONG, 0, tag, MPI_COMM_WORLD);
        unsigned long gapResult = mpz_get_ui(largestGap);
        MPI_Send(&gapResult, 1, MPI_UNSIGNED_LONG, 0, tag, MPI_COMM_WORLD);            
    }
    MPI_Finalize();
    return 0;
}