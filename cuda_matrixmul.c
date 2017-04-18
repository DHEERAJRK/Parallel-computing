#include <stdio.h>
#include <math.h>
#define TILE_WIDTH 2

// shared
__global__ void
MatrixMul( float *Md , float *Nd , float *Pd , const int WIDTH )
{
	__shared__ float Mds [TILE_WIDTH][TILE_WIDTH] ;
    __shared__ float Nds [TILE_WIDTH][TILE_WIDTH] ;

         // calculation of thread id
          unsigned int col = TILE_WIDTH*blockIdx.x + threadIdx.x ;
          unsigned int row = TILE_WIDTH*blockIdx.y + threadIdx.y ;

	for (int m = 0 ; m<WIDTH/TILE_WIDTH ; m++ )
    {
		Mds[threadIdx.y][threadIdx.x] =  Md[row*WIDTH + (m*TILE_WIDTH + threadIdx.x)]  ;
		Nds[threadIdx.y][threadIdx.x] =  Nd[ ( m*TILE_WIDTH + threadIdx.y) * WIDTH + col] ;
		// for syncronizeing the threads
		__syncthreads() ; 

	 // Do for each tile
	   for ( int k = 0; k<TILE_WIDTH ; k++ )
				   Pd[row*WIDTH + col]+= Mds[threadIdx.x][k] * Nds[k][threadIdx.y] ;
		// for syncronizeing the threads
	 __syncthreads() ; 

	}
}

int main ()
{
   const int WIDTH = 6 ;
   float array1_h[WIDTH][WIDTH] ,array2_h[WIDTH][WIDTH],
                     result_array_h[WIDTH][WIDTH] ,M_result_array_h[WIDTH][WIDTH]  ;
  float *array1_d , *array2_d ,*result_array_d  ,*M_result_array_d ; // device array
  int i , j ;
  //input in host array
  for ( i = 0 ; i<WIDTH ; i++ )
  {
     for (j = 0 ; j<WIDTH ; j++ )
     {
        array1_h[i][j] = 1 ;
        array2_h[i][j] = 2 ;
     }
  }

  //create device array cudaMalloc
  cudaMalloc((void **) &array1_d , WIDTH*WIDTH*sizeof (int) ) ;
  cudaMalloc((void **) &array2_d , WIDTH*WIDTH*sizeof (int) ) ;

  //copy host array to device array

  cudaMemcpy ( array1_d , array1_h , WIDTH*WIDTH*sizeof (int) , cudaMemcpyHostToDevice ) ;
  cudaMemcpy ( array2_d , array2_h , WIDTH*WIDTH*sizeof (int) , cudaMemcpyHostToDevice ) ;

  //allocating memory for resultent device array

  cudaMalloc((void **) &result_array_d , WIDTH*WIDTH*sizeof (int) ) ;
  cudaMalloc((void **) &M_result_array_d , WIDTH*WIDTH*sizeof (int) ) ;

  //calling kernal

  dim3 dimGrid ( WIDTH/TILE_WIDTH , WIDTH/TILE_WIDTH ,1 ) ;
  dim3 dimBlock( TILE_WIDTH, TILE_WIDTH, 1 ) ;
   MatrixMul<<<dimGrid,dimBlock>>> ( array1_d , array2_d ,M_result_array_d , WIDTH) ;
  cudaMemcpy(M_result_array_h , M_result_array_d , WIDTH*WIDTH*sizeof(int) ,cudaMemcpyDeviceToHost) ;

  for ( i = 0 ; i<WIDTH ; i++ )
  {
      for ( j = 0 ; j < WIDTH ; j++ )
     {
        printf ("%f   ",M_result_array_h[i][j] ) ;
     }
 printf ("\n") ;
}
 system("pause") ;
}
