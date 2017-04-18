# include <math.h>
# include <mpi.h>
# include <stdio.h>
# include <time.h>
# define DEBUG             0
# define CHUNKSIZE      1000
# define RANDOM_SEED       0
# define NEED_NUMBERS      1
# define RANDOM_NUMBERS    2

int main ( int argc, char *argv[] );
void timestamp ( );

int main ( int argc, char *argv[] )
{
  double calculatedPi,error,tolerance,wtime,x,y;
  int done,i,ierr,in,max,my_id,numprocs,out,randServer,request,temp,totalin,totalout;
  MPI_Status mesgStatus;
  int point_max = 1000000;
  int randNums[CHUNKSIZE];
  int ranks[1];
  MPI_Group worker_group;
  MPI_Comm workers;
  MPI_Group world_group;

  ierr = MPI_Init ( &argc, &argv );
//  Get the number of processors.
  ierr = MPI_Comm_size ( MPI_COMM_WORLD, &numprocs );
// Get the rank of this processor.
  ierr = MPI_Comm_rank ( MPI_COMM_WORLD, &my_id );

  if ( my_id == 0 ) 
  {
    timestamp ( );
  }

  if ( my_id == 0 )
  {
    wtime = MPI_Wtime ( );
  }

  ierr = MPI_Bcast ( &tolerance, 1, MPI_DOUBLE_PRECISION, 0,
    MPI_COMM_WORLD );

  printf ( "  Process %d is active.\n", my_id );
  ierr = MPI_Comm_group ( MPI_COMM_WORLD, &world_group );  
  randServer = numprocs-1;
  ranks[0] = randServer;
  ierr = MPI_Group_excl ( world_group, 1, ranks, &worker_group );
  ierr = MPI_Comm_create ( MPI_COMM_WORLD, worker_group, &workers );
  ierr = MPI_Group_free ( &worker_group );

  if ( my_id == randServer ) 
  {
# if RANDOM_SEED
    struct timeval time;
    gettimeofday( &time, 0 ); 
//  Initialize the random number generator 
    srandom ( (int)(time.tv_usec*1000000+time.tv_sec) );
# endif
    do
    {
      ierr = MPI_Recv ( &request, 1, MPI_INT, MPI_ANY_SOURCE, NEED_NUMBERS,
        MPI_COMM_WORLD, &mesgStatus );
      if ( request ) 
      {
        for ( i = 0; i < CHUNKSIZE; i++) 
        {
          randNums[i] = random();
        }
        ierr = MPI_Send ( randNums, CHUNKSIZE, MPI_INT, 
          mesgStatus.MPI_SOURCE, RANDOM_NUMBERS, MPI_COMM_WORLD );
      }
    } while ( 0 < request );

  }
  else  
  {
    request = 1;
    done = in = out = 0;
    max = 2147483647;

    ierr = MPI_Send ( &request, 1, MPI_INT, randServer, NEED_NUMBERS, 
      MPI_COMM_WORLD );

    while (!done) 
    {
      request = 1;
      ierr = MPI_Recv ( randNums, CHUNKSIZE, MPI_INT, randServer,
        RANDOM_NUMBERS, MPI_COMM_WORLD, &mesgStatus );

      for ( i = 0; i < CHUNKSIZE; ) 
      {
        x = ( ( float ) randNums[i++] ) / max;
        y = ( ( float ) randNums[i++] ) / max;

        if ( x * x + y * y < 1.0 ) 
        {
          in++;
        } 
        else 
        {
          out++;
        }

      }
	  temp = in;
      ierr = MPI_Reduce ( &temp, &totalin, 1, MPI_INT, MPI_SUM, 0, workers );
/* 
  Count total of ins.
*/ 
      temp = out;
      ierr = MPI_Reduce ( &temp, &totalout, 1, MPI_INT, MPI_SUM, 0, workers );
/* 
  Count total of outs.
*/
      if ( my_id == 0 ) 
      {
        calculatedPi = ( 4.0 * totalin ) / ( totalin + totalout );
        printf( "pi = %23.20lf\n", calculatedPi );

        if ( done )
        {
          request = 0;
        }
        else
        {
          request = 1;
        }

        ierr = MPI_Send ( &request, 1, MPI_INT, randServer, NEED_NUMBERS,
          MPI_COMM_WORLD );

        ierr = MPI_Bcast ( &done, 1, MPI_INT, 0, workers );
      } 
      else
      {
        ierr = MPI_Bcast ( &done, 1, MPI_INT, 0, workers );

        if ( !done ) 
        {
          request = 1;
          ierr = MPI_Send ( &request, 1, MPI_INT, randServer, NEED_NUMBERS,MPI_COMM_WORLD );
        }
      }
    }
  }

  if ( my_id == 0 ) 
  {
    wtime = MPI_Wtime ( ) - wtime;
    printf ( "\n" );
    printf ( "  Elapsed wallclock time = %g seconds.\n", wtime );
  }
/*
  Terminate MPI.
*/
  ierr = MPI_Finalize();
/*
  Terminate.
*/
  if ( my_id == 0 )
  {
    timestamp ( );
  }
  return 0;
}
void timestamp ( )
{
# define TIME_SIZE 40

  static char time_buffer[TIME_SIZE];
  const struct tm *tm;
  time_t now;

  now = time ( NULL );
  tm = localtime ( &now );
  return;
# undef TIME_SIZE
}
