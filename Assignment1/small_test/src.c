#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <assert.h>
#include "mpi.h"

FILE* fptr;

// function for stencil computation

void calculate(int rank,int N, int P, double buf1[],double buf2[],double buf3[],double buf4[],double** arr,double** new_arr)
{
  for (int i=1;i<N-1;i++)  // for all the elements which are not the boundary elements no matter which is the process id.
   {
     for (int j=1;j<N-1;j++)
     {
      new_arr[i][j] = (arr[i][j-1] + arr[i][j+1] + arr[i-1][j] + arr[i+1][j])/4;
     } 
   }

  
  if (rank == 0) // for the first process in the grid i.e for the process with rank 0
  {
    new_arr[0][0] = (arr[0][1] + arr[1][0]) /2; // for corner element at 0th posisiton
    new_arr[0][N-1] = (arr[0][N-2] + arr[1][N-1] + buf4[0]) /3; // for corner element at (0,N-1)
    new_arr[N-1][0] = (arr[N-2][0] + arr[N-1][1] + buf2[0]) /3; // for corner element 
    new_arr[N-1][N-1] = (arr[N-1][N-2] + arr[N-2][N-1] + buf2[N-1] + buf4[N-1])/4; //for bottom right corner element

    for (int i=1;i<N-1;i++) //first row apart from the first and last element (corner elements)
    {
      new_arr[0][i] = (arr[0][i-1] + arr[0][i+1] + arr[1][i])/3; // only three neighbours
    }

    for (int i=1;i<N-1;i++) //last row apart from the first and last element
    {
      new_arr[N-1][i] = (arr[N-1][i-1] + arr[N-1][i+1] + arr[N-2][i] + buf2[i])/4; 
    }

    for (int i=1;i<N-1;i++) //first column apart from the first and last element
    {
      new_arr[i][0] = (arr[i-1][0] + arr[i+1][0] + arr[i][1])/3; 
    }

    for (int i=1;i<N-1;i++) //last column apart from the first and last element
    {
      new_arr[i][N-1] = (arr[i-1][N-1] + arr[i+1][N-1] + arr[i][N-2] + buf4[i])/4;
    }
  }


  else if (rank == P-1) // for the corner process of the grid with the rank P-1. (where P is the sqrt of the total no. of processes)
  {
    new_arr[0][0] = (arr[0][1] + arr[1][0] + buf3[0]) /3; // for top left corner element
    new_arr[0][N-1] = (arr[0][N-2] + arr[1][N-1]) /2; // for top right corner elemet
    new_arr[N-1][0] = (arr[N-2][0] + arr[N-1][1] + buf3[N-1] + buf2[0]) /4; // for bottom left corner elements
    new_arr[N-1][N-1] = (arr[N-1][N-2] + arr[N-2][N-1] + buf2[N-1])/3; // for bottom right corner element
    
    for (int i=1;i<N-1;i++) //first row apart from the first and last element
    {
      new_arr[0][i] = (arr[0][i-1] + arr[0][i+1] + arr[1][i])/3;
    }

    for (int i=1;i<N-1;i++) //last row apart from the first and last element
    {
      new_arr[N-1][i] = (arr[N-1][i-1] + arr[N-1][i+1] + arr[N-2][i] + buf2[i])/4;
    }

    for (int i=1;i<N-1;i++) //first column apart from the first and last element
    {
      new_arr[i][0] = (arr[i-1][0] + arr[i+1][0] + arr[i][1] + buf3[i])/4;
    }

    for (int i=1;i<N-1;i++) //last column apart from the first and last element
    {
      new_arr[i][N-1] = (arr[i-1][N-1] + arr[i+1][N-1] + arr[i][N-2])/3;
    }
  }
  
  else if (rank == P*P-P) // for the bottom left corner process
  {
    new_arr[0][0] = (arr[0][1] + arr[1][0] + buf1[0]) /3; // for top left corner element
    new_arr[0][N-1] = (arr[0][N-2] + arr[1][N-1] + buf1[N-1] + buf4[0]) /4; // for top right corner element
    new_arr[N-1][0] = (arr[N-2][0] + arr[N-1][1]) /2; // for bottom left corner element 
    new_arr[N-1][N-1] = (arr[N-1][N-2] + arr[N-2][N-1] + buf4[N-1])/3; // for bottom right corner element
    
    for (int i=1;i<N-1;i++) //first row apart from the first and last element
    {
      new_arr[0][i] = (arr[0][i-1] + arr[0][i+1] + arr[1][i] + buf1[i])/4;
    }

    for (int i=1;i<N-1;i++) //last row apart from the first and last element
    {
      new_arr[N-1][i] = (arr[N-1][i-1] + arr[N-1][i+1] + arr[N-2][i])/3;
    }

    for (int i=1;i<N-1;i++) //first column apart from the first and last element
    {
      new_arr[i][0] = (arr[i-1][0] + arr[i+1][0] + arr[i][1])/3;
    }

    for (int i=1;i<N-1;i++) //last column apart from the first and last element
    {
      new_arr[i][N-1] = (arr[i-1][N-1] + arr[i+1][N-1] + arr[i][N-2] + buf4[i])/4;
    }
  }
  
  else if (rank == P*P-1)  //bottom right corner process
  {
    new_arr[0][0] = (arr[0][1] + arr[1][0] + buf1[0] + buf3[0]) /4; //top left corner element
    new_arr[0][N-1] = (arr[0][N-2] + arr[1][N-1] + buf1[N-1]) /3; // top right corner element
    new_arr[N-1][0] = (arr[N-2][0] + arr[N-1][1] + buf3[N-1]) /3; //bottom left corner elements
    new_arr[N-1][N-1] = (arr[N-1][N-2] + arr[N-2][N-1])/2; // bottom right corner element 

    for (int i=1;i<N-1;i++) //first row apart from the first and last element
    {
      new_arr[0][i] = (arr[0][i-1] + arr[0][i+1] + arr[1][i] + buf1[i])/4;
    }

    for (int i=1;i<N-1;i++) //last row apart from the first and last element
    {
      new_arr[N-1][i] = (arr[N-1][i-1] + arr[N-1][i+1] + arr[N-2][i])/3;
    }

    for (int i=1;i<N-1;i++) //first column apart from the first and last element
    {
      new_arr[i][0] = (arr[i-1][0] + arr[i+1][0] + arr[i][1] + buf3[i])/4;
    }

    for (int i=1;i<N-1;i++) //last column apart from the first and last element
    {
      new_arr[i][N-1] = (arr[i-1][N-1] + arr[i+1][N-1] + arr[i][N-2])/3;
    }
  }


  else if (rank < P) // all processes in first row other than the corner processes
  {
    new_arr[0][0] = (arr[0][1] + arr[1][0] + buf3[0]) /3;
    new_arr[0][N-1] = (arr[0][N-2] + arr[1][N-1] + buf4[0]) /3;
    new_arr[N-1][0] = (arr[N-2][0] + arr[N-1][1] + buf3[N-1] + buf2[0]) /4;
    new_arr[N-1][N-1] = (arr[N-1][N-2] + arr[N-2][N-1] + buf2[N-1] + buf4[N-1])/4;

    for (int i=1;i<N-1;i++) //first row apart from the first and last element
    {
      new_arr[0][i] = (arr[0][i-1] + arr[0][i+1] + arr[1][i])/3;
    }

    for (int i=1;i<N-1;i++) //last row apart from the first and last element
    {
      new_arr[N-1][i] = (arr[N-1][i-1] + arr[N-1][i+1] + arr[N-2][i] + buf2[i])/4;
    }

    for (int i=1;i<N-1;i++) //first column apart from the first and last element
    {
      new_arr[i][0] = (arr[i-1][0] + arr[i+1][0] + arr[i][1] + buf3[i])/4;
    }

    for (int i=1;i<N-1;i++) //last column apart from the first and last element
    {
      new_arr[i][N-1] = (arr[i-1][N-1] + arr[i+1][N-1] + arr[i][N-2] + buf4[i])/4;
    }
  }

  
 else if (rank > P*P- P /*&& rank < P*P-1*/)  // all processes in last row other than the corner processes
  {
    new_arr[0][0] = (arr[0][1] + arr[1][0] + buf3[0] + buf1[0]) /4;
    new_arr[0][N-1] = (arr[0][N-2] + arr[1][N-1] + buf1[N-1] + buf4[0]) /4;
    new_arr[N-1][0] = (arr[N-2][0] + arr[N-1][1] + buf3[N-1]) /3;
    new_arr[N-1][N-1] = (arr[N-1][N-2] + arr[N-2][N-1] + buf4[N-1])/3;

    for (int i=1;i<N-1;i++) //first row apart from the first and last element
    {
      new_arr[0][i] = (arr[0][i-1] + arr[0][i+1] + arr[1][i] + buf1[i])/4;
    }

    for (int i=1;i<N-1;i++) //last row apart from the first and last element
    {
      new_arr[N-1][i] = (arr[N-1][i-1] + arr[N-1][i+1] + arr[N-2][i])/3;
    }

    for (int i=1;i<N-1;i++) //first column apart from the first and last element
    {
      new_arr[i][0] = (arr[i-1][0] + arr[i+1][0] + arr[i][1] + buf3[i])/4;
    }

    for (int i=1;i<N-1;i++) //last column apart from the first and last element
    {
      new_arr[i][N-1] = (arr[i-1][N-1] + arr[i+1][N-1] + arr[i][N-2] + buf4[i])/4;
    }
  }

  else if (rank%P == 0)  // all processes in first column other than the corner processes
  {
    new_arr[0][0] = (arr[0][1] + arr[1][0] + buf1[0]) /3;
    new_arr[0][N-1] = (arr[0][N-2] + arr[1][N-1] + buf1[N-1] + buf4[0]) /4;
    new_arr[N-1][0] = (arr[N-2][0] + arr[N-1][1] + buf2[0]) /3;
    new_arr[N-1][N-1] = (arr[N-1][N-2] + arr[N-2][N-1] + buf4[N-1] + buf2[N-1])/4;

    for (int i=1;i<N-1;i++) //first row apart from the first and last element
    {
      new_arr[0][i] = (arr[0][i-1] + arr[0][i+1] + arr[1][i] + buf1[i])/4;
    }

    for (int i=1;i<N-1;i++) //last row apart from the first and last element
    {
      new_arr[N-1][i] = (arr[N-1][i-1] + arr[N-1][i+1] + arr[N-2][i] + buf2[i])/4;
    }

    for (int i=1;i<N-1;i++) //first column apart from the first and last element
    {
      new_arr[i][0] = (arr[i-1][0] + arr[i+1][0] + arr[i][1])/3;
    }

    for (int i=1;i<N-1;i++) //last column apart from the first and last element
    {
      new_arr[i][N-1] = (arr[i-1][N-1] + arr[i+1][N-1] + arr[i][N-2] + buf4[i])/4;
    }
  }

  else if ((rank+1)%P == 0)  // all processes in last column other than the corner processes
  {
    new_arr[0][0] = (arr[0][1] + arr[1][0] + buf1[0] + buf3[0]) /4;
    new_arr[0][N-1] = (arr[0][N-2] + arr[1][N-1] + buf1[N-1]) /3;
    new_arr[N-1][0] = (arr[N-2][0] + arr[N-1][1] + buf2[0] + buf3[N-1]) /4;
    new_arr[N-1][N-1] = (arr[N-1][N-2] + arr[N-2][N-1] + buf2[N-1])/3;

    for (int i=1;i<N-1;i++) //first row apart from the first and last element
    {
      new_arr[0][i] = (arr[0][i-1] + arr[0][i+1] + arr[1][i] + buf1[i])/4;
    }

    for (int i=1;i<N-1;i++) //last row apart from the first and last element
    {
      new_arr[N-1][i] = (arr[N-1][i-1] + arr[N-1][i+1] + arr[N-2][i] + buf2[i])/4;
    }

    for (int i=1;i<N-1;i++) //first column apart from the first and last element
    {
      new_arr[i][0] = (arr[i-1][0] + arr[i+1][0] + arr[i][1] + buf3[i])/4;
    }

    for (int i=1;i<N-1;i++) //last column apart from the first and last element
    {
      new_arr[i][N-1] = (arr[i-1][N-1] + arr[i+1][N-1] + arr[i][N-2])/3;
    }
  }

  else   // all middle processes
  {
    new_arr[0][0] = (arr[0][1] + arr[1][0] + buf1[0] + buf3[0]) /4;
    new_arr[0][N-1] = (arr[0][N-2] + arr[1][N-1] + buf1[N-1] + buf4[0]) /4;
    new_arr[N-1][0] = (arr[N-2][0] + arr[N-1][1] + buf2[0] + buf3[N-1]) /4;
    new_arr[N-1][N-1] = (arr[N-1][N-2] + arr[N-2][N-1] + buf2[N-1]+buf4[N-1])/4;

    for (int i=1;i<N-1;i++) //first row apart from the first and last element
    {
      new_arr[0][i] = (arr[0][i-1] + arr[0][i+1] + arr[1][i] + buf1[i])/4;
    }

    for (int i=1;i<N-1;i++) //last row apart from the first and last element
    {
      new_arr[N-1][i] = (arr[N-1][i-1] + arr[N-1][i+1] + arr[N-2][i] + buf2[i])/4;
    }

    for (int i=1;i<N-1;i++) //first column apart from the first and last element
    {
      new_arr[i][0] = (arr[i-1][0] + arr[i+1][0] + arr[i][1] + buf3[i])/4;
    }

    for (int i=1;i<N-1;i++) //last column apart from the first and last element
    {
      new_arr[i][N-1] = (arr[i-1][N-1] + arr[i+1][N-1] + arr[i][N-2] + buf4[i])/4;
    }
  }
}


// function for multiple send and receive
double sndrcv(int rank, int N, int P, double** arr, double** new_arr)
{

	MPI_Status status;

	double buf1[N], buf2[N], buf3[N], buf4[N];

	double stime = MPI_Wtime();

	for (int q = 0; q < 50; ++q) {
       //communication

    // for sending the data row wise, i.e processes will send the data to the process below it in the grid and receive the data from the process above it, and in the second go process will send data to the processes above it in the grid and process will receive the data from 2the process below it.
    
    int rank_up = rank-P ; // to identify which process is above that process
    int rank_down = rank+P; //to identify which process is below that process

    //for top to bottom communication

    if( rank_up >=0 && rank_up < P*P) // if the rank lies between the 0 and maximum rank of the process
      {
        for (int i = 0; i < N; ++i) {

          MPI_Recv(buf1+i,1,MPI_DOUBLE,rank_up,rank,MPI_COMM_WORLD,&status); // receiving the data from the process above it.          
        }
   

      }
    if (rank_down >=0 && rank_down < P*P) { // if the rank lies between the 0 and maximum rank of the process

      for (int i = 0; i < N; ++i) {

        MPI_Send(&arr[N-1][i],1,MPI_DOUBLE,rank_down,rank_down,MPI_COMM_WORLD); // sending the data to the process below it
        
      }

      
    }
    

    //for bottom to  top communication
    
    if (rank_down >=0 && rank_down < P*P) { // if the rank lies between the 0 and maximum rank of the process

      for (int i = 0; i < N; ++i) {

        MPI_Recv(buf2+i,1,MPI_DOUBLE,rank_down,rank,MPI_COMM_WORLD,&status); // receiving  the data from the process below it
                 
      }

      
    }
	
  

    if( rank_up >=0 && rank_up < P*P) // if the rank lies between the 0 and maximum rank of the process
      {
        for (int i = 0; i < N; ++i) {

          MPI_Send(&arr[0][i],1,MPI_DOUBLE,rank_up,rank_up,MPI_COMM_WORLD); // sending the data to the process above it.          
        }
   

      }
   	
    //

    int rank_left = rank-1;
    int rank_right = rank+1;

    //for left to right communication

    if( rank_left >=0 && rank_left < P*P) // if the rank lies between the 0 and maximum rank of the process
      {

        if(rank%P!=0){ // left  boundary processes will not receive any data from process with rank: rank - 1 

          for (int i = 0; i < N; ++i) {

          MPI_Recv(buf3+i,1,MPI_DOUBLE,rank_left,rank,MPI_COMM_WORLD,&status); // receiving the data from the process left to it.          
         }
        }
      
   

      }
    if (rank_right >=0 && rank_right < P*P) {

      if ((rank + 1)%P !=0 ) { //right boundary processes will not send any data to the processes with the rank: rank+1

        for (int i = 0; i < N; ++i) {

        MPI_Send(&arr[i][N-1],1,MPI_DOUBLE,rank_right,rank_right,MPI_COMM_WORLD); // sending the data to the process right to it
        
        }

      }
            
    }
    

    //for right to left communication
    
    if (rank_right >=0 && rank_right < P*P) {

      if ((rank+1)%P!=0) { // right boundary processes will not receive any data from the processes with the rank : rank+1

        
        for (int i = 0; i < N; ++i) {

          MPI_Recv(buf4+i,1,MPI_DOUBLE,rank_right,rank,MPI_COMM_WORLD,&status); // receiving  the data from the process right to it
                 
        }

        
      }
      
    }
	
  

    if( rank_left >=0 && rank_left < P*P) // if the rank lies between the 0 and maximum rank of the process
      {
        if (rank%P!=0) { //Left boundary processes will not send data to the processes with the rank :rank-1 

          for (int i = 0; i < N; ++i) {

            MPI_Send(&arr[i][0],1,MPI_DOUBLE,rank_left,rank_left,MPI_COMM_WORLD); // sending the data to the process left to it.          
          }
         
       }
   
    }    


    //Computation

    //double new_arr[N][N];
    
    calculate (rank,N,P,buf1,buf2,buf3,buf4,arr,new_arr); // calling function calculate to perform the stencil computation

    //memcpy(arr, new_arr, sizeof(double)*N*N);
    for (int i=0;i<N;i++)
    {
    	for (int j=0;j<N;j++) arr[i][j] = new_arr[i][j];
    }

   }

    double etime = MPI_Wtime();
	double time = etime-stime, maxTime;
	MPI_Reduce (&time, &maxTime, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

	return maxTime;
}


//function for pack and unpack
double pckunpck(int rank, int N, int P, double** arr, double** new_arr)
{
	MPI_Status status;
	int position;
    double buffer[N] , recvBuf[N];

    double buf1[N], buf2[N], buf3[N], buf4[N];

    double stime = MPI_Wtime();

	for (int q = 0; q < 50; ++q) 
    {
       //communication
    

    // for sending the data row wise, i.e processes will send the data to the process below it in the grid and receive the data from the process above it, and in the second go process will send data to the processes above it in the grid and process will receive the data from 2the process below it.
    
    int rank_up = rank-P ; // to identify which process is above that process
    int rank_down = rank+P; //to identify which process is below that process

    //for top to bottom communication

    if( rank_up >=0 && rank_up < P*P) // if the rank lies between the 0 and maximum rank of the process
      {
       // receiving the data from the process above it.          
        MPI_Recv(recvBuf,N*sizeof(double),MPI_PACKED,rank_up,rank,MPI_COMM_WORLD,&status);
        position = 0;
        for (int i = 0; i < N; ++i) {

          MPI_Unpack(recvBuf,N*sizeof(double),&position,&buf1[i],1,MPI_DOUBLE,MPI_COMM_WORLD);
        }
   

      }
    if (rank_down >=0 && rank_down < P*P) { // if the rank lies between the 0 and maximum rank of the process

      position = 0;
      for (int i = 0; i < N; ++i) {

        // double buffer[N];
        MPI_Pack(&arr[N-1][i],1,MPI_DOUBLE,buffer,N*sizeof(double),&position,MPI_COMM_WORLD);
        
      }

     MPI_Send(buffer,position,MPI_PACKED,rank_down,rank_down,MPI_COMM_WORLD); // sending the data to the process below it

      
    }
    

    //for bottom to  top communication
    
    if (rank_down >=0 && rank_down < P*P) { // if the rank lies between the 0 and maximum rank of the process

       MPI_Recv(recvBuf,N*sizeof(double),MPI_PACKED,rank_down,rank,MPI_COMM_WORLD,&status); // receiving  the data from the process below it
       position = 0;
       for (int i = 0; i < N; ++i) {

         MPI_Unpack(recvBuf,N*sizeof(double),&position,&buf2[i],1,MPI_DOUBLE,MPI_COMM_WORLD);
                 
      }

      
    }
	
  

    if( rank_up >=0 && rank_up < P*P) // if the rank lies between the 0 and maximum rank of the process
      {
        position  = 0;
        for (int i = 0; i < N; ++i) {

          MPI_Pack(&arr[0][i],1,MPI_DOUBLE,buffer,N*sizeof(double),&position,MPI_COMM_WORLD);
        }
       
       MPI_Send(buffer,position,MPI_PACKED,rank_up,rank_up,MPI_COMM_WORLD); // sending the data to the process above it
   

      }
   	
    //

    int rank_left = rank-1;
    int rank_right = rank+1;
    
    //for left to right communication

    if( rank_left >=0 && rank_left < P*P) // if the rank lies between the 0 and maximum rank of the process
      {

        if(rank%P!=0){ // left  boundary processes will not receive any data from process with rank: rank - 1 


          MPI_Recv(recvBuf,N*sizeof(double),MPI_PACKED,rank_left,rank,MPI_COMM_WORLD,&status); // receiving the data from the process left to it.

          position = 0;
          for (int i = 0; i < N; ++i) {

            MPI_Unpack(recvBuf,N*sizeof(double),&position, &buf3[i],1,MPI_DOUBLE,MPI_COMM_WORLD );
         }
        }
      
   

      }
    if (rank_right >=0 && rank_right < P*P) {

      if ((rank + 1)%P !=0 ) { //right boundary processes will not send any data to the processes with the rank: rank+1

        position = 0;
        for (int i = 0; i < N; ++i) {

          MPI_Pack(&arr[i][N-1],1, MPI_DOUBLE,buffer,N*sizeof(double),&position,MPI_COMM_WORLD);
        
        }

       MPI_Send(buffer,position,MPI_PACKED,rank_right,rank_right,MPI_COMM_WORLD); // sending the data to the process right to it

      }
            
    }
    

    //for right to left communication
    
    if (rank_right >=0 && rank_right < P*P) {

      if ((rank+1)%P!=0) { // right boundary processes will not receive any data from the processes with the rank : rank+1

        
        MPI_Recv(recvBuf,N*sizeof(double),MPI_PACKED,rank_right,rank,MPI_COMM_WORLD,&status); // receiving  the data from the process right to it

        position  = 0;
        for (int i = 0; i < N; ++i) {

          MPI_Unpack(recvBuf,N*sizeof(double),&position,&buf4[i],1,MPI_DOUBLE,MPI_COMM_WORLD);  
                 
        }

        
      }
      
    }
	
  

    if( rank_left >=0 && rank_left < P*P) // if the rank lies between the 0 and maximum rank of the process
      {
        if (rank%P!=0) { //Left boundary processes will not send data to the processes with the rank :rank-1 

          position = 0;  
          for (int i = 0; i < N; ++i) {

            MPI_Pack(&arr[i][0],1,MPI_DOUBLE,buffer,N*sizeof(double),&position,MPI_COMM_WORLD);
                     
          }
          MPI_Send(buffer,position,MPI_PACKED,rank_left,rank_left,MPI_COMM_WORLD); // sending the data to the process left to it.
       }
   
    } 

    //Computation

    //double new_arr[N][N];
    
     calculate (rank,N,P,buf1,buf2,buf3,buf4,arr,new_arr); // calling function calculate to perform the stencil computation

     //memcpy(arr, new_arr, sizeof(double)*N*N);
     for (int i=0;i<N;i++)
    {
    	for (int j=0;j<N;j++) arr[i][j] = new_arr[i][j];
    }

     }

    double etime = MPI_Wtime();
	double time = etime-stime, maxTime;
	MPI_Reduce (&time, &maxTime, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

	return maxTime;
}

// function for derived datatype (vector)
double datatyp (int rank, int N, int P, double** arr, double ** new_arr)
{
	MPI_Status status;
	MPI_Datatype rowtype,columntype;

    MPI_Type_vector(N,1,1,MPI_DOUBLE, &rowtype); // derived data type for one row.
    MPI_Type_commit(&rowtype); //committing the new datatype
     

    MPI_Type_vector(N,1,N,MPI_DOUBLE,&columntype); //derived datatype for one column
    MPI_Type_commit(&columntype); //committing the new datatype


    double buf1[N], buf2[N], buf3[N], buf4[N];

    double stime = MPI_Wtime();

	for (int q = 0; q < 50; ++q) {

      //communication

     // for sending the data row wise, i.e processes will send the data to the process below it in the grid and receive the data from the process above it, and in the second go process will send data to the processes above it in the grid and process will receive the data from 2the process below it.
    
    int rank_up = rank-P ; // to identify which process is above that process
    int rank_down = rank+P; //to identify which process is below that process

    //for top to bottom communication

    if( rank_up >=0 && rank_up < P*P) // if the rank lies between the 0 and maximum rank of the process
      {
       

         MPI_Recv(&buf1,N,MPI_DOUBLE,rank_up,rank,MPI_COMM_WORLD,&status); // receiving the data from the process above it.          
        
   

      }

    if (rank_down >=0 && rank_down < P*P) { // if the rank lies between the 0 and maximum rank of the process


        MPI_Send(&arr[N-1][0],1,rowtype,rank_down,rank_down,MPI_COMM_WORLD); // sending the data to the process below it
        
     

      
    }
    

    //for bottom to  top communication
    
    if (rank_down >=0 && rank_down < P*P) { // if the rank lies between the 0 and maximum rank of the process

      

        MPI_Recv(&buf2,N,MPI_DOUBLE,rank_down,rank,MPI_COMM_WORLD,&status); // receiving  the data from the process below it
                 
      

      
    }
  
  

    if( rank_up >=0 && rank_up < P*P) // if the rank lies between the 0 and maximum rank of the process
      {
       

          MPI_Send(&arr[0][0],1,rowtype,rank_up,rank_up,MPI_COMM_WORLD); // sending the data to the process above it.          
        
   

      }
    
    //

    int rank_left = rank-1;
    int rank_right = rank+1;

    //for left to right communication

    if( rank_left >=0 && rank_left < P*P) // if the rank lies between the 0 and maximum rank of the process
      {

        if(rank%P!=0){ // left  boundary processes will not receive any data from process with rank: rank - 1 

         

          MPI_Recv(&buf3,N,MPI_DOUBLE,rank_left,rank,MPI_COMM_WORLD,&status); // receiving the data from the process left to it.          
         
        }
      
   

      }
    if (rank_right >=0 && rank_right < P*P) {

      if ((rank + 1)%P !=0 ) { //right boundary processes will not send any data to the processes with the rank: rank+1

       

        MPI_Send(&arr[0][N-1],1,columntype,rank_right,rank_right,MPI_COMM_WORLD); // sending the data to the process right to it
        
        
      }
            
    }
    

    //for right to left communication
    
    if (rank_right >=0 && rank_right < P*P) {

      if ((rank+1)%P!=0) { // right boundary processes will not receive any data from the processes with the rank : rank+1

        
        

          MPI_Recv(&buf4,N,MPI_DOUBLE,rank_right,rank,MPI_COMM_WORLD,&status); // receiving  the data from the process right to it
                 
        

        
      }
      
    }
  
  

    if( rank_left >=0 && rank_left < P*P) // if the rank lies between the 0 and maximum rank of the process
      {
        if (rank%P!=0) { //Left boundary processes will not send data to the processes with the rank :rank-1 

          
            MPI_Send(&arr[0][0],1,columntype,rank_left,rank_left,MPI_COMM_WORLD); // sending the data to the process left to it.          
          
         
       }
   
    }    


    //Computation

    calculate (rank,N,P,buf1,buf2,buf3,buf4,arr,new_arr); // calling function calculate to perform the stencil computation

    //memcpy(arr, new_arr, sizeof(double)*N*N);
    for (int i=0;i<N;i++)
    {
    	for (int j=0;j<N;j++) arr[i][j] = new_arr[i][j];
    }
    }

    double etime = MPI_Wtime();
    double time = etime-stime, maxTime;
    MPI_Reduce (&time, &maxTime, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    MPI_Type_free(&rowtype);
    MPI_Type_free(&columntype);

    return maxTime;
} 

 

// main 
int main(int argc, char* argv[])
{
	//Initializations
	MPI_Init(&argc, &argv);
	int N = atoi(argv[1]); // data points are N*N;

	int rank, size;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	int P;
	for (int i=0;i<size;i++) 
	{
		if (i*i==size) 
		{
			P=i;   // if no. of process = K then P is sqrt of k
			break;
		}
	}

	if (!rank)
	{
		char file_prefix[20];
		snprintf(file_prefix, 15, "data%d.txt", size);
		fptr = fopen(file_prefix, "a");
	}

	/*

    // dynamically declaring auxiliary array for stencil computationm
    double  **new_arr = (double **)malloc(N * sizeof(double  *));

    for (int i=0; i<N; i++) 
         new_arr[i] = (double *)malloc(N * sizeof(double)); 
    

    // declaring dynamically 2D contiguous array
     
      

    // dynamically declaring array for stencil computationm
    double  **arr = (double **)malloc(N * sizeof(double  *));

    for (int i=0; i<N; i++) 
         arr[i] = (double *)malloc(N * sizeof(double)); 

         */

	double *a = malloc(N*N * sizeof(double));
    double **arr = malloc(N * sizeof(double*));
     for(int i = 0; i < N; i++) 
       arr[i] = a + N*i;

   double *b = malloc(N*N * sizeof(double));
    double **new_arr = malloc(N * sizeof(double*));
     for(int i = 0; i < N; i++) 
       new_arr[i] = b + N*i;
   


    //////////////////  MULTIPLE SENDS AND RECEIVES  //////////////////////////

    
     for (int i=0;i<N;i++)   // randomly initializing  2D array of datapoints
	{
		for (int j=0;j<N;j++)
		{
		  //arr[i][j] = (double)(rand())/RAND_MAX;
                 arr[i][j] = rank*(i+j);
		}
	}
     
	

    double maxTime = sndrcv(rank, N, P, arr, new_arr); // calling function for multiple send and receive (blocking calls)

    
  	if (!rank) 
  		{
  			printf ("Time taken for Multiple Send_Receives : %lf \n",maxTime);
  			fprintf (fptr, "Send_Receive, N = %d, time = %lf \n",N, maxTime);

        printf("Send/Recv\n");
      for (int i=0;i<N;i++)
      {
        for (int j=0;j<N;j++) printf("%lf ", arr[i][j]);
          printf("\n");
      }
  		}

      

  	



  	//////////////////  MPI_PACK and MPI_UNPACK  //////////////////////////


  	for (int i=0;i<N;i++)   // randomly initializing  2D array of datapoints
	{
		for (int j=0;j<N;j++)
		{
		  //arr[i][j] = (double)(rand())/RAND_MAX;
                   arr[i][j] = rank*(i+j);
		}
	}
    

    maxTime = pckunpck(rank, N, P, arr, new_arr);
     
  	if (!rank) 
  		{
  			printf ("Time taken for Pack_Unpack : %lf \n", maxTime);
  			fprintf (fptr, "Pack_Unpack, N = %d, time = %lf \n",N, maxTime);

        printf("Pack/Unpack\n");
      for (int i=0;i<N;i++)
      {
        for (int j=0;j<N;j++) printf("%lf ", arr[i][j]);
          printf("\n");
      }
  		}

      





  	//////////////////  MPI_Datatype usage   //////////////////////////


  	for (int i=0;i<N;i++)   // randomly initializing  2D array of datapoints
	{
		for (int j=0;j<N;j++)
		{
		  //arr[i][j] = (double)(rand())/RAND_MAX;
                   arr[i][j] = rank*(i+j);
		}
	}
     

    maxTime = datatyp(rank, N, P, arr, new_arr);

    if (!rank) 
    	{
    		printf ("Time taken for MPI_Datatype : %lf \n", maxTime);
    		fprintf (fptr, "Datatype, N = %d, time = %lf \n",N, maxTime);

    		fclose(fptr);

        printf("Datatype\n");
      for (int i=0;i<N;i++)
      {
        for (int j=0;j<N;j++) printf("%lf ", arr[i][j]);
          printf("\n");
      }

    	}

      

    MPI_Finalize();

    


	return 0;
}
