/* ************************************************************************************
 * Name: Hugh Phung
 * Student number: 3842508
 * Year: 2021
 * Project: The Dining Philosopher's Problem


 * Method:
 * Create an array of 'chopsticks', say five chopsticks
 * Create and array of threads, the same amount as chopsticks
 * For the required program, say 10 seconds,
 * sleep ('think') for a random number of milliseconds and try to grab
 * the first chopstick (chopstick[1]) for philosopher 1 and (chopstick[2])
 * for philosopher 2.
 * If it isn't available, sleep the thread until the chopstick is ready.
 * (Suggested solution was to sleep the philosopher for a random time and try again,
 * but it seemed more efficient to sleep the thread on a condition wait.)
 * Do the same for the second chopstick.
 * One both locks have been obtained, sleep ('eat') for a random number of 
 * milliseconds and then release both locks.

 * Limitations:
 * Philosophers don't get fair meals if the program runs for long periods of time.
 * ************************************************************************************
 */

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>

#define MAX_PHILOSOPHER 5
#define MAX_CHOPSTICK 5
#define MAX_RUN_TIME 10

// Using usleep() to sleep thread which takes in microseconds
// 1000 milliseconds
#define MAX_EAT_TIME 1000000
// 2000 milliseconds
#define MAX_THINK_TIME 2000000

// States for Philosophers
#define THINKING 0
#define EATING 1
#define WAITING 2
#define PACKING_UP 3

pthread_mutex_t chopstick[MAX_CHOPSTICK];
pthread_cond_t condChopstick[MAX_CHOPSTICK];

int philosopherSTATES[MAX_PHILOSOPHER];
int philosopherEatCounter[MAX_PHILOSOPHER];

bool stop = false;

void printPhilosopherEatCount()
{
    printf("\nPhilosopher eat counter: \n");
    printf("Philosopher[%d]: %d\n", MAX_PHILOSOPHER-MAX_PHILOSOPHER, philosopherEatCounter[MAX_PHILOSOPHER-MAX_PHILOSOPHER]);
    int i;
    for (i = 1; i < MAX_PHILOSOPHER-1; ++i)
    {
        printf("Philosopher[%d]: %d\n", i, philosopherEatCounter[i]);
    }
    printf("Philosopher[%d]: %d\n", MAX_PHILOSOPHER-1, philosopherEatCounter[i]);

}
void* philosophise(void* var)
{   
    int* philosopherNumPtr = (int*) var;
    int philosopherNum = *philosopherNumPtr;

    while (!stop)
    {
        // Let philosopher think for random time under 2000 milliseconds
        philosopherSTATES[philosopherNum] = THINKING;
        printf("Philosopher %d is thinking...\n", philosopherNum);
        int thinkTime = rand() % MAX_THINK_TIME;
        usleep(thinkTime);
        
        /*
         * Left will refer to the number one less than that the philospher and vice versa with the right.
         * The chopstick on the left of a philosopher will always have the same number as the philosopher;
         * For instance, left chopstick of philosopher[3] is chopstick[3] and the right is chopstick[4].
         */
        int left = (philosopherNum+(MAX_PHILOSOPHER-1)) % MAX_PHILOSOPHER;
        int right = (philosopherNum + 1) % MAX_PHILOSOPHER;

        /* 
         * Applies to any philosopher that isn't the very last one in the circle
         * In this case they will pick up the left chopstick first, then the right.
         * The last philosopher will wait on the right chopstick first, and then the left
         * to break the circular loop that would inevitably cause a deadlock.
         */
        if (philosopherNum < right)
        {
            /*
             * While the philosopher on the left side is eating, it will still have its right chopstick.
             * The exception is the last philosopher who takes the right chopstick first, so in this case
             * they could be in WAITING state.
             * Therefore these instances the chopstick on the left will not be available and so will sleep in the while loop.
             */
            pthread_mutex_lock(&chopstick[philosopherNum]);
            while(philosopherSTATES[left] == EATING ||
                ((left == (MAX_PHILOSOPHER-1)) && philosopherSTATES[left] == WAITING) || (philosopherSTATES[left] == PACKING_UP))
                  {
                      printf("Philosopher %d is waiting on their first chopstick[%d]...\n", philosopherNum, philosopherNum);
                      pthread_cond_wait(&condChopstick[philosopherNum], &chopstick[philosopherNum]);
                  }

            printf("Philosopher %d picking up first (left) chopstick[%d]...\n", philosopherNum, philosopherNum);
            philosopherSTATES[philosopherNum] = WAITING;

            // Thinks for a bit before getting the next chopstick as per assignment specifications 
            printf("Philosopher %d is thinking in WAITING state for a bit until they're confidant enough to try for the second chopstick.\n", philosopherNum);
            int thinkTime = rand() % MAX_THINK_TIME;
            usleep(thinkTime);  

            /*
             * When the chopstick to the right is free, that's when the philosopher to the right is THINKING or WAITING.
             * However the exception here is that because the last philosopher (philosopher[4]) picks up the right chopstick first
             * to break the circular loop, when philosopher[4] is waiting for their left chopstick, philosopher[3] can still yoink it.
             * Therefore philosopher[3] doesn't have to sleep in the following while loop if philosopher[4] is in waiting state.
             * Hence for all other occurrences philosophers will sleep if the philosopher to the right is in waiting state or still eating.
             */
            pthread_mutex_lock(&chopstick[right]);
            while (((philosopherSTATES[right] == WAITING) && ((right) != (MAX_PHILOSOPHER-1))) || (philosopherSTATES[right] == EATING))
            {
                printf("Philosopher %d is about to pass out because they're waiting on chopstick[%d]\n", philosopherNum, right);
                pthread_cond_wait(&condChopstick[right], &chopstick[right]);
            }
            printf("Philosopher %d picking up second (right) chopstick[%d]...\n", philosopherNum, right);
        }

        // Applies to last philosopher, in this case it's always philosopher[4] since the philosopher to the right is smaller.
        else
        {
            /* 
             * Waiting on the right chopstick first.
             * Right chopstick (chopstick[0]) will be available once philosopher[0] is thinking or when they are in the process of 
             * putting their chopsticks down - in this case they're putting down their left one first.
             * Therefore must wait when philosopher[0] is either EATING or WAITING.
             */
            pthread_mutex_lock(&chopstick[right]);
            while((philosopherSTATES[right] == EATING) || (philosopherSTATES[right] == WAITING))
                  {
                      printf("Philosopher %d is waiting on their first chopstick[%d]...\n", philosopherNum, right);
                      pthread_cond_wait(&condChopstick[right], &chopstick[right]);
                  }
            printf("Philosopher %d picking up first (right) chopstick[%d]...\n", philosopherNum, right);
            philosopherSTATES[philosopherNum] = WAITING;

            // Thinks for a bit before getting the next chopstick as per assignment specifications
            printf("Philosopher %d is thinking in WAITING state for a bit until they're confidant enough to try for the second chopstick.\n", philosopherNum);
            int thinkTime = rand() % MAX_THINK_TIME;
            usleep(thinkTime);  

            /* 
             * Exits this while loop when philosopher to the left (philosopher[3]) thinking or waiting since that's when philosopher[4]
             * can pick up chopstick[3].
             * Therefore must wait when philosopher[3] is either EATING or still PACKING_UP.
             */
            pthread_mutex_lock(&chopstick[philosopherNum]);
            while ((philosopherSTATES[left] == EATING) || (philosopherSTATES[left] == PACKING_UP))
            {
                printf("Philosopher %d is about to pass out because they're waiting on chopstick[%d]\n", philosopherNum, philosopherNum);
                pthread_cond_wait(&condChopstick[philosopherNum], &chopstick[philosopherNum]);
            }
            printf("Philosopher %d picking up second (left) chopstick[%d]...\n", philosopherNum, philosopherNum);
        }

        // Once both chopsticks have been acquired the philosopher can begin FEASTING YUMMY YUM YUM
        philosopherSTATES[philosopherNum] = EATING;
        ++philosopherEatCounter[philosopherNum];
        printf("Philosopher %d is eating... HA HA FINALLY FOOD IS MINE!\n", philosopherNum);
        int eatTime = rand() % MAX_EAT_TIME;
        usleep(eatTime);
        printf("Philosopher %d has finished eating.\n", philosopherNum);

        /* 
         * The philosopher can only put down one chopstick at a time because they've bitten off more than they could chew
         * They always put down the left chopstick first, then the right.
         * Changes state to PACKING_UP and signals to let any other philosopher waiting on their left chopstick pick it up.
         * Finally they let go off their right chopstick.
         */
        printf("Philosopher %d putting down first (left) chopstick[%d]. Packing up...\n", philosopherNum, philosopherNum);
        philosopherSTATES[philosopherNum] = PACKING_UP;
        pthread_mutex_unlock(&chopstick[philosopherNum]);
        pthread_cond_signal(&condChopstick[philosopherNum]);

        printf("Philosopher %d putting down second (right) chopstick[%d]. Done.\n", philosopherNum, right);
        pthread_mutex_unlock(&chopstick[right]);
        pthread_cond_signal(&condChopstick[right]);

    }

    // As philosophers exit, state is set back to THINKING to absolve any dependency deadlocks
    philosopherSTATES[philosopherNum] = THINKING;
    printf("\033[0;32mPhilosopher %d is EXITING. DON'T COME BACK!\033[0m\n", philosopherNum);
    return EXIT_SUCCESS;
}

int main(void)
{
    // Initialising random time
    srand(time(NULL));

    // 5 philosopher threads
    pthread_t philosopher[MAX_PHILOSOPHER];

    // Creating an array to pass through elements
    int philosopherNum[MAX_PHILOSOPHER];

    int i;

    // Initialise all philosophers to start off in THINKING state
    for (i = 0; i < MAX_PHILOSOPHER; ++i)
    {
        philosopherSTATES[i] = THINKING;
    }

    // Initialising chopstick mutexes
    for (i = 0; i < MAX_CHOPSTICK; ++i)
    {
        if (pthread_mutex_init(&chopstick[i], NULL) != 0)
        {
            fprintf(stderr, "Failed to initialise chopstick mutex\n");
        }
    }

    // Initialising chopstick conditions
    for (i = 0; i < MAX_CHOPSTICK; ++i)
    {
        if (pthread_cond_init(&condChopstick[i], NULL) != 0)
        {
            fprintf(stderr, "Failed to initialise condition variable\n");
        }
    }
    
    // Initialising philosophers to begin their threads
    for (i = 0; i < MAX_PHILOSOPHER; ++i)
    {
        philosopherNum[i] = i;
        if (pthread_create(&philosopher[i], NULL, &philosophise, &philosopherNum[i]) != 0)
        {
            fprintf(stderr, "Failed to create Philosopher thread\n");
        }
    }

    // Let program run for 10 seconds
    sleep(MAX_RUN_TIME);
    stop = true;

    // Joining philosophers
    for (i = 0; i < MAX_PHILOSOPHER; ++i)
    {
        if (pthread_join(philosopher[i], NULL) != 0)
        {
            fprintf(stderr, "Failed to join Philosopher thread\n");
        }
    }

    // Cleaning mutexes
    for (i = 0; i < MAX_CHOPSTICK; ++i)
    {
        if (pthread_mutex_destroy(&chopstick[i]) != 0)
        {
            fprintf(stderr, "Failed to destroy chopstick mutex\n");
        }
    }

    // Cleaning conditions
    for (i = 0; i < MAX_CHOPSTICK; ++i)
    {
        if (pthread_cond_destroy(&condChopstick[i]) != 0)
        {
            fprintf(stderr, "Failed to destroy chopstick condition\n");
        }
    }

    
    printf("\nTIME IS UP. All philosophers BETTER HAVE LEFT MY RESTAURANT.\n");

    printPhilosopherEatCount();

    return EXIT_SUCCESS;

}
