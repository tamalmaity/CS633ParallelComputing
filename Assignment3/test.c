#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <assert.h>
#include <float.h>
#include "mpi.h"

FILE* fptr;


// main 
int main(int argc, char* argv[])
{
  //Initializations
  MPI_Init(&argc, &argv);

  int rank, size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  double** arr;
  int rows,col;

  ////////////////////////////// FILE READING STARTS //////////////////////////////

  if (!rank)
  {
    fptr = fopen("tdata.csv", "r");
    char str[1000]; //assuming maximum 1000 characters per line

    ///Find out the number of rows and columns
    rows = 0, col=0;

    while (fgets(str, sizeof(str), fptr))
    {
      rows++;
      if(rows==1)
      {
        int i=0;
        while (str[i]!='\n')
        {
          if (str[i+1]=='\n' || str[i]==',') //criteria to change whatever is in temp to double
          {
            col++;
          }

          i++;
        }
      }
    }
    rows--; //since first row is column name in csv
    col=col-2; // since the first two are latitude and longitude

    fseek(fptr,0, SEEK_SET); //to reset the fptr to beginning of file
    //printf("%d %d\n", rows, col);
    
    double *a = malloc(rows* col* sizeof(double));
    arr = malloc(rows * sizeof(double*));
    

    for(int i = 0; i < rows; i++) 
      arr[i] = a + col*i;

    int count = 0;


    while (fgets(str, sizeof(str), fptr))
    {
      count++;
      if (count == 1) continue; // to not store the column headers
      //printf("%s\n", str);
      int j =0;
      //while (str[j]!='\n') printf("%c ", str[j++]);
  
  
      char temp[10]; // assuming maximum 10 characters per value
      int arr_index = 0, i=0, k=0;
      while (str[i]!='\n')
      {
        if (str[i+1]=='\n' || str[i]==',') //criteria to change whatever is in temp to double
        { 

          
          if (str[i+1]=='\n') temp[k] = str[i];
          //printf("%s\n", temp);
          if(arr_index==0||arr_index==1) 
           arr_index++;
          else
          {
            arr[count-2][arr_index-2] = atof(temp);
            arr_index++;
          } 
          memset(temp, 0, 10*sizeof(char));
          k=0;
        }
        else temp[k++] = str[i]; //appending character by character

        i++;
      }

    }
    //printf("chikchik\n");
   
    fclose(fptr);
 

    //test if the 2-d array values are correctly stored
    
    /*for(int i=0;i<rows;i++)
    {
      for (int j=0;j<col;j++) printf("%lf ", arr[i][j]);
      printf("\n\n\n\n\n");
    }*/

  }


  ////////////////////////////// FILE READING ENDS //////////////////////////////


  double stime = MPI_Wtime();

  MPI_Bcast(&col, 1, MPI_INT, 0, MPI_COMM_WORLD);
 
  // Create the datatype
  MPI_Datatype double_type;

  MPI_Type_contiguous(col, MPI_DOUBLE, &double_type); //communicating data  in chunks of one row
  MPI_Type_commit(&double_type);

  double global_min, *local_min, *gather_local_min;
  int info[size];// send_count[size], displ[size];

  if (!rank)
  {

    //array to store how much data each rank computes
    info[0] = rows/2;
    int rem = rows-info[0];
    for (int i=1;i<size;i++)
    {
      if(i==size-1) info[i] = rem;
      else info[i] = rem/2;
      rem = rem - (rem/2);
    }
    //for (int i=0;i<size;i++) send_count[i] = info[i];
    for (int i=size-1;i>1;i--) info[i-1]+= info[i];

    //for (int i=0;i<size;i++) printf("%d ", info[i]);
  }
  
  double time0;

  int to_recv;
  MPI_Scatter(info, 1, MPI_INT, &to_recv, 1, MPI_INT, 0, MPI_COMM_WORLD);
  if (!rank)
  {

    int row_begin = rows/2;
    int s = rows-row_begin;
    MPI_Request request1[s];
    MPI_Status status1[s];


    //sending second part of data array to process1
    for(int i = row_begin; i<rows; i++)
     { 
       MPI_Isend(&arr[i][0], 1, double_type, 1, (i-row_begin), MPI_COMM_WORLD,&request1[i-row_begin]); // rank 0 sending data to 1 
     }

     ///COMPUTATION
     double stime0 = MPI_Wtime();
     local_min = malloc(col*sizeof(double));

     for (int i=0;i<col;i++)
     {
        double minm = DBL_MAX;
        for (int j=0;j<row_begin;j++)
        {
          if (arr[j][i]<minm) minm = arr[j][i];
        }
        local_min[i] = minm;
     }
     double etime0 = MPI_Wtime();
     time0 = etime0-stime0;

     //displ[0] = 0;
     //for (int i=1;i<size;i++) displ[i] = displ[i-1] + send_count[i-1];

     //Wait for all Isends to other process to complete

     MPI_Waitall(s,request1,status1);
     //printf("Hello Rank 0 \n");

   gather_local_min = malloc(col*size*sizeof(double));
   
     
  }



  if (rank==1)
  {
    int s = to_recv;

    MPI_Request request1[s];
    MPI_Status status1[s];

    double *b = malloc(s* col* sizeof(double));
    double** temp = malloc(s * sizeof(double*));
    for(int i = 0; i < s; i++) 
      temp[i] = b + col*i;

    for(int j = 0; j< s; j++)
     {
        MPI_Irecv(&temp[j][0], 1 , double_type, 0, j, MPI_COMM_WORLD, &request1[j]); // root is receiving data from all the other processes
     } 
     MPI_Waitall(s,request1,status1);
     //printf("Hello Rank 1 recv \n");


     
     //Sending rows,col info and data to process2
     int s_new = s- (s/2);
     MPI_Request request2[s_new];
     MPI_Status status2[s_new];

    //sending second part of data array to process2
    for(int i = s/2; i<s; i++)
     { 
       MPI_Isend(&temp[i][0], 1, double_type, 2, (i-(s/2)), MPI_COMM_WORLD,&request2[i-(s/2)]); // rank 1 sending data to 2 
     }

     //printf("from 1 %d\n",s-s_new); 
    
     //COMPUTATION
     //printf("%d\n",col);

     local_min = malloc(col *sizeof(double));

     for (int i=0;i<col;i++)
     {
        double minm = DBL_MAX;
        for (int j=0;j<s/2;j++)
        {
          if (temp[j][i]<minm) minm = temp[j][i];
        }
        local_min[i] = minm;
     }

     //Wait for all Isends to other process to complete
     
    // printf("Yoyo\n");
     MPI_Waitall(s_new,request2,status2);
     //printf("Hello Rank 1 send \n");

     //printf("Hello\n");
     //Send computed data to parent process
     //MPI_Send(local_min, s_new, MPI_DOUBLE, 0, 65, MPI_COMM_WORLD);

  }


  if (rank==2)
  {
    int s = to_recv;

    MPI_Request request1[s];
    MPI_Status status1[s];

    double *b = malloc(s* col* sizeof(double));
    double** temp = malloc(s * sizeof(double*));
    for(int i = 0; i < s; i++) 
      temp[i] = b + col*i;
   
        //printf("from 2 %d\n",s); 

      
    for(int j = 0; j< s; j++)
     {
       //printf("%d ", j);
        MPI_Irecv(&temp[j][0], 1 , double_type, 1, j, MPI_COMM_WORLD, &request1[j]); // root is receiving data from all the other processes
     } 
     MPI_Waitall(s,request1,status1);
    
     //printf("Hello Rank 2 recv \n");



     //COMPUTATION
     local_min = malloc(col*sizeof(double));


     for (int i=0;i<col;i++)
     {
        double minm = DBL_MAX;
        for (int j=0;j<s;j++)
        {

          if (temp[j][i]<minm) 
          {

            minm = temp[j][i];
          
          }       

        }
        local_min[i] = minm;
     }


     //Send computed data to parent process
     //MPI_Send(local_min, s, MPI_DOUBLE, 0, 67, MPI_COMM_WORLD);

  }



  //int to_send = to_recv;
  //if (rank!=0 || rank!=(size-1)) to_send=to_recv/2;

  MPI_Gather(local_min, col, MPI_DOUBLE, gather_local_min, col, MPI_DOUBLE, 0, MPI_COMM_WORLD);
  
  //printf("Gather by 0 \n");

    
  if(!rank)
  {
    global_min = DBL_MAX;
    for (int i=0;i<col;i++)
    {
      double minm = DBL_MAX;
      for(int j=0;j<size;j++)
      {
        if (gather_local_min[j*col+i]<minm) minm = gather_local_min[j*col+i];
      }

      local_min[i] = minm;
      if (global_min>local_min[i]) global_min = local_min[i];
      //printf("%d %lf \n", i, local_min[i]);
    }
    //printf("%lf " , global_min);
  }

  double etime = MPI_Wtime();

  double time = etime - stime;

  double maxtime;
  MPI_Reduce (&time, &maxtime, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

  if(!rank) 
    {
      printf("The time taken for rank 0 computation is : %lf\n", time0);
      printf("The total time taken is : %lf\n", maxtime);
    }
   
    
  MPI_Finalize();
  return 0;
}
