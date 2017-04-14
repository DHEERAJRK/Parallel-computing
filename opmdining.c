#include <omp.h>        //Use OpenMP
#include <stdio.h>      //Provide I/O
#include <stdlib.h>     //Use NULL
#include <unistd.h>     //Use usleep (sleep for a number of microseconds)

#define NUM_PHIL 5      //The number of philosophers
#define MEALS 100        //The number of meals per philosopher

static omp_lock_t chopsticks[NUM_PHIL];   //Locks to represent chopsticks

/*  This function represents the philosopher threads.  Each philosopher must
 *  wait until all others have started before continuing.  Each then eats NUM_PHIL
 *  meals.  Philosophers must acquire chopsticks to their left and right before eating.
 */
void philosopher()
{
  //Wait for all threads to start
  #pragma omp barrier

  //Set up variables for the thread id, right chopstick, and left chopstick.
  int id = omp_get_thread_num();
  int right_chopstick;
  int left_chopstick;

  //Philosophers 0 through NUM_PHIL-2 reach to the left first then the right.
  if(id < NUM_PHIL -1)
  {
    right_chopstick = id;
    left_chopstick = id+1;
  }
  //Philosopher NUM_PHIL-1 has its left and right swapped.  Notice that it reaches in the
  //opposite order of all other philosophers.
  else
  {
    right_chopstick = 0;
    left_chopstick = id;
  }

  //Acquire chopsticks (semaphores), eat, wait for 100 microseconds, then release 
  //chopsticks (semaphores).
  int i;
  for(i = 0; i < MEALS; i++)
  {
    omp_set_lock(&chopsticks[left_chopstick]);
    omp_set_lock(&chopsticks[right_chopstick]);

    printf("philosopher %d is eating\n", id);
    usleep(100);

    omp_unset_lock(&chopsticks[left_chopstick]);
    omp_unset_lock(&chopsticks[right_chopstick]);
  }
}

//Main function.
int main(int argc, char ** argv)
{
  int i;

  //Initialize locks
  for(i = 0; i < NUM_PHIL; i++)
    omp_init_lock(&chopsticks[i]);

  //Create and start philosopher threads.
  #pragma omp parallel num_threads(NUM_PHIL)
  {
    philosopher();
  }

  //Wait for philosophers to finish then destroy locks.
  for(i = 0; i < NUM_PHIL; i++)
    omp_destroy_lock(&chopsticks[i]);

  //End program.
  return 0;
}