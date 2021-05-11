#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"

int main( int argc, char *argv[])
{
  //4 processes per group. Total processes = 24
  //2 processes per group for Intra group and 2 each for Inter group
  //Senders and receivers in each group aren't repeated to avoid contention..
  //.. as much as possible


  int myrank, recvCount;
  int N1 = 16*1024/8;
  int N2 = 2048*1024/8;
  double data_first[N1];
  double data_sec[N2]; 
  MPI_Status status;

  MPI_Init( &argc, &argv );
  MPI_Comm_rank( MPI_COMM_WORLD, &myrank );

  //Data Initialization 
  for (int i=0; i<N1; i++)
    data_first[i] = i;

  for (int i=0; i<N2; i++)
    data_sec[i] = i;


  //INTRA GROUP with size N1
  
  double stime = MPI_Wtime();
  if (myrank%4==0)   //Intra group senders
  {
    
    MPI_Send(data_first, N1, MPI_DOUBLE, myrank+1, 99, MPI_COMM_WORLD);
    //printf("meow1\n");
  }
  else if (myrank%4==1)  //Intra group receivers
  {
    
    MPI_Recv(data_first, N1, MPI_DOUBLE, myrank-1, 99, MPI_COMM_WORLD, &status);
    //printf("meow2\n");
  }
  double etime = MPI_Wtime();
  double intra_time1 = (etime-stime); //time for data of size N1


  //INTER GROUP with size N1

  stime = MPI_Wtime();
  if (myrank%4==2)   //Inter group senders
  {
    int to = myrank+5;
    if (to>23) to=3;
    
    MPI_Send(data_first, N1, MPI_DOUBLE, to, 33, MPI_COMM_WORLD);
    //printf("meow3\n");
  }
  else if (myrank%4==3)  //Inter group receivers
  {
    int from = myrank-5;
    if (from<0) from = 22;
    
    MPI_Recv(data_first, N1, MPI_DOUBLE, from, 33, MPI_COMM_WORLD, &status);
    //printf("meow4\n");
  }
  etime = MPI_Wtime();
  double inter_time1 = (etime-stime); //time for data of size N1


  //printf("Before barrier\n");

  MPI_Barrier(MPI_COMM_WORLD); //Wait for all processes to come before going... 
                  //..to send data of different size so as to not overlap time

  //printf("After barrier\n");


  //INTRA GROUP with size N2
  
  stime = MPI_Wtime();
  if (myrank%4==0)   //Intra group senders
  {
    //printf("meow5\n");
    MPI_Send(data_sec, N2, MPI_DOUBLE, myrank+1, 999, MPI_COMM_WORLD);
  }
  else if (myrank%4==1)  //Intra group receivers
  {
   // printf("meow6\n");
    MPI_Recv(data_sec, N2, MPI_DOUBLE, myrank-1, 999, MPI_COMM_WORLD, &status);
  }
  etime = MPI_Wtime();
  double intra_time2 = (etime-stime); //time for data of size N2



  //INTER GROUP with size N2

  stime = MPI_Wtime();
  if (myrank%4==2)   //Inter group senders
  {
    int to = myrank+5;
    if (to>23) to=3;
    //printf("meow7\n");
    MPI_Send(data_sec, N2, MPI_DOUBLE, to, 333, MPI_COMM_WORLD);
  }
  else if (myrank%4==3)  //Inter group receivers
  {
    int from = myrank-5;
    if (from<0) from = 22;
   // printf("meow8\n");
    MPI_Recv(data_sec, N2, MPI_DOUBLE, from, 333, MPI_COMM_WORLD, &status);
  }
  etime = MPI_Wtime();
  double inter_time2 = (etime-stime); //time for data of size N2


  double t1,t2,t3,t4;
  MPI_Reduce (&intra_time1, &t1, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
  MPI_Reduce (&inter_time1, &t2, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
  MPI_Reduce (&intra_time2, &t3, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
  MPI_Reduce (&inter_time2, &t4, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

  //t1 = 6* [2*x + N1*y] ----> x is the per hop Latency and y is Bandwidth
  //t2 = 6* [4*x + N1*y]
  //t3 = 6* [2*x + N2*y]
  //t4 = 6* [4*x + N2*y]

  if (!myrank)
  {
    t1 = t1/6; t2 = t2/6; t3 = t3/6; t4 = t4/6;
    printf("Per hop Latency when data is 16KB : %lf\n", (t2-t1)/2);
    printf("Per hop Latency when data is 2048KB : %lf\n", (t4-t3)/2);
    printf("Bandwidth while Intra group comm is : %lf\n", (2048-16)/((t3-t1)*1024));
    printf("Bandwidth while Inter group comm is : %lf\n", (2048-16)/((t4-t2)*1024));

  }


  MPI_Finalize();
  return 0;

}

