#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>

#define NUM_MESSAGES 3*NUM_PHILOSOPHERS*NUM_DINNERS

const int NUM_DINNERS;
const int NUM_PHILOSOPHERS;
const int NEED_LEFT_CHOPSTICK = 0;
const int NEED_RIGHT_CHOPSTICK = 1;
const int RELEASE_CHOPSTICKS = 2;

void philosopher(int rank)
{
  int i;
  int leftChopstick;
  int rightChopstick;
  int dummy;
  
  if(rank == NUM_PHILOSOPHERS)
  {

    //***************************************************************
    //Compute the index of the left and right chopsticks here
    //and place them in leftChopstick and rightChopstick respectively

  }
  else
  {
    leftChopstick = rank;
    rightChopstick = rank - 1;
  }

  for(i = 0; i < NUM_DINNERS; i++)
  {
    printf("Philosopher %d is hungry for meal %d.\n", rank, i);
    /* Get chopsticks */
    MPI_Send(&leftChopstick, 1, MPI_INT, 0, NEED_LEFT_CHOPSTICK, MPI_COMM_WORLD);
    MPI_Recv(&dummy, 1, MPI_INT, 0, NEED_LEFT_CHOPSTICK, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    /* Eat dinner */
    printf("Philosopher %d is eating meal %d.\n", rank, i);

    MPI_Send(&dummy, 1, MPI_INT, 0, RELEASE_CHOPSTICKS, MPI_COMM_WORLD);
  }  
}

void server()
{
  int i;
  int j;
  int count;
  int dummy;
  int leftChopstick;
  int rightChopstick;
  int * needsLeftChopstick;
  int * needsRightChopstick;

  MPI_Request * requests;
  MPI_Status * statuses;
  int * index;
  int * rightRequest;
  int * leftRequest;
  int * chopsticks;

  requests = (MPI_Request *) malloc(sizeof(MPI_Request)*NUM_MESSAGES);
  statuses = (MPI_Status *) malloc(sizeof(MPI_Status)*NUM_MESSAGES);
  index = (int *) malloc(sizeof(int)*NUM_MESSAGES);

  rightRequest = (int *) malloc(sizeof(int)*NUM_PHILOSOPHERS*NUM_DINNERS);
  leftRequest = (int *) malloc(sizeof(int)*NUM_PHILOSOPHERS*NUM_DINNERS);

  chopsticks = (int *) malloc(sizeof(int)*NUM_PHILOSOPHERS);
  needsLeftChopstick = (int *) malloc(sizeof(int)*NUM_PHILOSOPHERS);
  needsRightChopstick = (int *) malloc(sizeof(int)*NUM_PHILOSOPHERS);

  for(i = 0; i < NUM_PHILOSOPHERS; i++)
  {
    chopsticks[i] = 1;
    needsLeftChopstick[i] = 0;
    needsRightChopstick[i] = 0;
  }

  int messagesHandled = 0;

  printf("Starting receive messages from philosophers.\n");
  printf("The server will receive %d messages.\n", NUM_MESSAGES);

  for(i = 0; i < NUM_PHILOSOPHERS; i++)
  {
    for(j = 0; j < NUM_DINNERS; j++)
    {
      /* Receive requests to acquire chopsticks */
      MPI_Irecv(&leftRequest[i*NUM_DINNERS+j], 1, MPI_INT, i+1, NEED_LEFT_CHOPSTICK, MPI_COMM_WORLD, &requests[i*NUM_DINNERS*3+j*3+0]);
      MPI_Irecv(&rightRequest[i*NUM_DINNERS+j], 1, MPI_INT, i+1, NEED_RIGHT_CHOPSTICK, MPI_COMM_WORLD, &requests[i*NUM_DINNERS*3+j*3+1]);

      /* Receive requests to release chopsticks */
      MPI_Irecv(&dummy, 1, MPI_INT, i+1, RELEASE_CHOPSTICKS, MPI_COMM_WORLD, &requests[i*NUM_DINNERS*3+j*3+2]);
    }
  }

  printf("Beginning service to philosophers.\n");

  while(messagesHandled < NUM_MESSAGES)
  {
    printf("The number of messages handled is %d.\n", messagesHandled);
    printf("Waiting for messages from philosophers.\n");
    MPI_Waitsome(NUM_MESSAGES, requests, &count, index, statuses);

    printf("Processing philosopher messages.\n");
    for(i = 0; i < count; i++)
    {
      /* Handle messages */
      printf("The server is handling %d messages.\n", count);
      j = index[i];
      printf("Handling message number %d from process %d with tag %d.\n", j, statuses[i].MPI_SOURCE, statuses[i].MPI_TAG);
      printf("Or is it process %d tag %d.\n", statuses[i].MPI_SOURCE, statuses[i].MPI_TAG);

      /* First message type is a request for a left chopstick */
      if( statuses[i].MPI_TAG == NEED_LEFT_CHOPSTICK )
      {
        if(statuses[i].MPI_SOURCE < NUM_PHILOSOPHERS)
          leftChopstick = statuses[i].MPI_SOURCE;
        else
          leftChopstick = statuses[i].MPI_SOURCE-1;

        if(chopsticks[leftChopstick] == 0)
          needsLeftChopstick[statuses[i].MPI_SOURCE-1] = 1;
        else
        {
          chopsticks[leftChopstick] = 0;
          MPI_Send(&leftChopstick, 1, MPI_INT, statuses[i].MPI_SOURCE, NEED_LEFT_CHOPSTICK, MPI_COMM_WORLD);
          messagesHandled++;
        }
      }
      /* Second message type is a request for a right chopstick */
      else if( statuses[i].MPI_TAG == NEED_RIGHT_CHOPSTICK )
      {
        if(statuses[i].MPI_SOURCE < NUM_PHILOSOPHERS)
          rightChopstick = statuses[i].MPI_SOURCE-1;
        else
          rightChopstick = 0;

        if(chopsticks[rightChopstick] == 0)
          needsRightChopstick[statuses[i].MPI_SOURCE-1] = 1;
        else
        {
          chopsticks[rightChopstick] = 0;
          MPI_Send(&rightChopstick, 1, MPI_INT, statuses[i].MPI_SOURCE, NEED_RIGHT_CHOPSTICK, MPI_COMM_WORLD);
          messagesHandled++;
        }
      }
      /* Third message type is a request to release chopsticks */
      else if ( statuses[i].MPI_TAG == RELEASE_CHOPSTICKS)
      {
        if(statuses[i].MPI_SOURCE < NUM_PHILOSOPHERS)
        {
          chopsticks[statuses[i].MPI_SOURCE - 1] = 1;
          chopsticks[statuses[i].MPI_SOURCE] = 1;
        }
        else
        {
          chopsticks[statuses[i].MPI_SOURCE - 1] = 1;
          chopsticks[0] = 1;
        }
        messagesHandled++;
      }
    }

    /* Check for needy philosophers */
    for(i = 0; i < NUM_PHILOSOPHERS; i++)
    {
      if(i+1 < NUM_PHILOSOPHERS)
      {
        if(needsRightChopstick[i] && chopsticks[i])
        {
          chopsticks[i] = 0;
          MPI_Send(&chopsticks[i], 1, MPI_INT, i+1, NEED_RIGHT_CHOPSTICK, MPI_COMM_WORLD);
          messagesHandled++;
          needsRightChopstick[i] = 0;
        }
        else if(needsLeftChopstick[i] && chopsticks[i+1])
        {
          chopsticks[i+1] = 0;
          MPI_Send(&chopsticks[i+1], 1, MPI_INT, i+1, NEED_LEFT_CHOPSTICK, MPI_COMM_WORLD);
          messagesHandled++;
          needsLeftChopstick[i] = 0;
        }
      }
      else
      {
//********************************************************************
//Handle the last philosopher here assuming in a similar fashion to the
//code above assuming that we are not allowing circular wait.
//********************************************************************
      }
    }
  }

  free( (void *) requests);
  free( (void *) statuses);
  free( (void *) index);

  free( (void *) rightRequest);
  free( (void *) leftRequest);

  free( (void *) chopsticks);
  free( (void *) needsLeftChopstick);
  free( (void *) needsRightChopstick);
}

int main(int argc, char ** argv)
{
  int i;
  int myrank;
  int npes;

  if(argc != 3)
    exit(0);
  
  sscanf(argv[1], "%d", &NUM_PHILOSOPHERS);
  printf("The number of philosophers is %d\n", NUM_PHILOSOPHERS);
  sscanf(argv[2], "%d", &NUM_DINNERS);
  printf("The number of times for each philosopher to eat is %d\n", NUM_DINNERS);


  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
  MPI_Comm_size(MPI_COMM_WORLD, &npes);

  if(myrank == 0)
  {
    double start = MPI_Wtime();
    server();
    double end = MPI_Wtime() - start;
    printf("The run time of this program was %lf seconds.\n", end);
  }
  else
  {
//*****************************************************************
//Call the philosopher function with the appropriate rank here
//*****************************************************************
  }

  MPI_Finalize();
}

