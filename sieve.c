/* Compute prime numbers via sieve of eratosthenes
 *
 * gcc sieve.c -lpthread
 *
 * ./a.out <largest_number> <num_threads>
 */


#include <stdio.h>
#include <pthread.h>
#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>


// Info for each thread
typedef struct tinfo
{
    int id;
    int base_number;
    int* numbers;
    int* evaluated_numbers;
    int largest_number;
} threadInfo;


// Time measure function by Julian Gutierrez
// Get time in milliseconds
double CLOCK()
{
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return (t.tv_sec * 1000) + (t.tv_nsec * 1e-6);
}


// Print primes found
void printPrimes(int* numbers, int length)
{
    int num_primes_found = 0;
    printf("Printing primes:\n");

    // Print the first prime
    printf("2\n");
    num_primes_found++;
    for (int i = 3; i < length; i += 2)
    {
        // If number is prime, count it and print it
        if (numbers[i])
        {
            printf("%i\n", i);
            num_primes_found++;
        }
    }
    
    // Print number of primes
    printf("Number of primes found: %i\n", num_primes_found);
}


// Function for each thread
// Compute the multiples of a base number and mark
// them off in an array
void* computeMultiples(void *tinfo)
{
    // Get thread info
    threadInfo *thread_info = (threadInfo*) tinfo;
    int base_number = thread_info->base_number;
    int largest_number = thread_info->largest_number;

    // Compute multiples of base number
    for (int i = base_number * base_number; i < largest_number + 1; i = i + base_number)
    {
        // Mark as not a prime
        thread_info->numbers[i] = 0;
    }
    
    // Cleanup
    free(thread_info);
    pthread_exit(NULL);
}


// Find primes up to largest_number using a maximum of num_threads
int* findPrimesParallel(int largest_number, int num_threads)
{
    // Initialize array of threads
    pthread_t *threads = malloc(sizeof(pthread_t) * num_threads);

    // Initialize array of numbers
    // Set all values to true
    // Numbers that are not prime will eventually be set to false
    int *numbers = malloc(sizeof(int) * (largest_number + 1));
    for (int i = 0; i < largest_number + 1; i++)
        numbers[i] = 1;

    // Keeping track of next prime to try
    // Starting at 3 since it's the first odd prime
    int lowest_prime = 3;
    
    // Keep spawning threads until all primes have been found
    int done = 0;
    while(!done)
    {
        // Counter to keep track of threads to join later
        int num_threads_started = 0;

        // Calculate the base numbers
        // Each thread will take a base and compute all multiples
        // Start threads
        int error;
        for (int i = 0; i < num_threads; i++)
        {
            // Create thread info
            threadInfo *thread_info = malloc(sizeof(threadInfo));

            // Set thread info
            thread_info->id = i;
            thread_info->numbers = numbers;
            thread_info->largest_number = largest_number;
            thread_info->base_number = -1;

            // Find the next prime to try
            // Increment by 2 to skip even numbers
            for (;lowest_prime < (int) ceil(sqrt(largest_number + 1)); lowest_prime += 2)
            {
                if (numbers[lowest_prime] == 1)
                {
                    thread_info->base_number = lowest_prime;
                    lowest_prime += 2;
                    break;
                }
            }

            // Couldn't find another number to evaluate
            // Break out of for loop and while loop
            if(thread_info->base_number == -1)
            {
                done = 1;
                break;
            }

            // Create thread
            error = pthread_create(&threads[i], NULL, computeMultiples, thread_info);
            num_threads_started++;

            // Quit if error
            if (error)
            {
                printf("PTHREAD ERROR: pthread_create() code: %d\n", error);
                exit(1);
            }
        }

        // Wait for threads
        for (int i=0; i < num_threads_started; i++)
        {
            pthread_join(threads[i], NULL);
        }
    }
   
    // Cleanup
    free(threads);

    return numbers;
}


int main(int argc, char* argv[])
{
    // Usage
    char* usage = "Usage: ./sieve <largest_number> <num_threads>";
   
    // Arg 
    if (argc < 3 || argc > 3)
    {
        printf("%s\n", usage);
        return 1;
    }

    // Setup timer variables
    double start_time, finish_time;

    // argv[1] is the largest number to compute up to
    int largest_number = atoi(argv[1]);

    // We need a number larger than 1 since we assume 2 is prime
    assert(largest_number > 1);

    // argv[2] is the number of threads to use
    int num_threads = atoi(argv[2]);

    // Get start time
    start_time = CLOCK();

    // Compute primes
    int *numbers = findPrimesParallel(largest_number, num_threads);
    
    // Get end time
    finish_time = CLOCK();

    // Print primes
    printPrimes(numbers, largest_number + 1);

    // Print time spent
    double total_time = finish_time - start_time;
    printf("Time: %f milliseconds\n", total_time);

    // Cleanup
    free(numbers);

    return 0;
}
