#include "mpi.h"
#include <stdlib.h>
#include <stdio.h>


int main( int argc, char **argv )
{
    int rank, size;

    MPI_Init( &argc, &argv );

    /* Create the buffer */
    MPI_Comm_size( MPI_COMM_WORLD, &size );
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );

    double *sbuf, *rbuf;
    int *sendcounts, *recvcounts, *rdispls, *sdispls;
    int i, j, N=4;

    sbuf = (double *)malloc( N * sizeof(double) );
    rbuf = (double *)malloc( size * N * sizeof(double) );
    
    /* Load up the buffers */
    for (i=0; i<N; i++) {
        sbuf[i] = i + 100*rank;
    }
    for (int i=0;i<N*size;i++)
    {
        rbuf[i] = -i;
    }

    /* Create and load the arguments to alltoallv */
    
    sendcounts = (int *)malloc( size * sizeof(int) );
    recvcounts = (int *)malloc( size * sizeof(int) );
    rdispls = (int *)malloc( size * sizeof(int) );
    sdispls = (int *)malloc( size * sizeof(int) );

    //int sendcounts[4], recvcounts[4], rdispls[4], sdispls[4];

    /*
    if (rank==0)//0,300,301,302
    {
      sendcounts = (int[4]){1,2,3,2};
      recvcounts = (int[4]){N,N,N,N};
      //rdispls = (int[4]){0,1,1,1};
      sdispls = (int[4]){0,0,0,0};
    }
    if (rank==1)//0,1,200,300,301
    {
      sendcounts = (int[4]){0,0,2,1};
      recvcounts = (int[4]){N,N,N,N};
      //rdispls = (int[4]){0,2,2,3};
      sdispls = (int[4]){0,0,0,0};
    }
    if (rank==2)//0,1,2,100,101,200,201,300
    {
      sendcounts = (int[4]){0,1,2,1};
      recvcounts = (int[4]){N,N,N,N};
      //rdispls = (int[4]){0,3,5,7};
      sdispls = (int[4]){0,0,0,0};
    }
    if (rank==3)//0,1,100,200,300
    {
      sendcounts = (int[4]){3,2,1,1};
      recvcounts = (int[4]){N,N,N,N};
      //rdispls = (int[4]){0,2,3,4};
      sdispls = (int[4]){0,0,0,0};
    }*/

    for (int i=0;i<N;i++)
    {
      int rand1 = rand()%N; 
      int rand2 = rand()%(N-rand1);
      sendcounts[i] = rand1;
      sdispls[i] = rand2;
    }

    int* temp = (int *)malloc(N *sizeof(int) );
    MPI_Alltoall(sendcounts, 1, MPI_INT, temp, 1, MPI_INT, MPI_COMM_WORLD);
    
    rdispls[0] = 0;
    int sum = 0;
    for (int i=1;i<N;i++)
    {
      sum+= temp[i-1];
      rdispls[i] = sum;
    }
    for (int i=0;i<N;i++) recvcounts[i] = temp[i];

    //for (int i=0;i<N;i++) printf("%d %d %d\n", rank, i, rdispls[i]);

    MPI_Alltoallv( sbuf, sendcounts, sdispls, MPI_DOUBLE,
                       rbuf, recvcounts, rdispls, MPI_DOUBLE, MPI_COMM_WORLD );
    
    int arr_size = 0;
    for (int i=0;i<size;i++) arr_size+= recvcounts[i];

    for (int i=0;i<arr_size;i++) 
    {
      printf("%d %d %lf\n", rank, i, rbuf[i]);
    }


    MPI_Finalize();
    return 0;
}