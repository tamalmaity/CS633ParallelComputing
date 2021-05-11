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

  float** arr;
  int rows,col;
  int dim[2], ROWS, COL;

  ////////////////////////////// FILE READING STARTS //////////////////////////////

  if (!rank)
  {
    fptr = fopen(argv[1], "r");
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
          if (str[i+1]=='\n' || str[i]==',') //criteria to change whatever is in temp to float
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
    
    float *a = malloc(rows* col* sizeof(float));
    arr = malloc(col * sizeof(float*));
    

    for(int i = 0; i < col; i++) 
      arr[i] = a + rows*i;

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
        if (str[i+1]=='\n' || str[i]==',') //criteria to change whatever is in temp to float
        { 

          
          if (str[i+1]=='\n') temp[k] = str[i];
          //printf("%s\n", temp);
          if(arr_index==0||arr_index==1) 
           arr_index++;
          else
          {
            arr[arr_index-2][count-2] = (float)atof(temp);
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

    //NOW WE HAVE col NUMBER OF ROWS AND rows NUMBER OF COLUMNS, so introducing new variables
    ROWS = col;
    COL = rows;
 

    //test if the 2-d array values are correctly stored
    
    /*for(int i=0;i<ROWS;i++)
    {
      for (int j=0;j<COL;j++) printf("%lf ", arr[i][j]);
      printf("\n\n\n\n\n");
    }*/

  }

  ////////////////////////////// FILE READING ENDS //////////////////////////////

  MPI_Barrier(MPI_COMM_WORLD);

  double stime = MPI_Wtime();

  dim[0] = ROWS;
  dim[1] = COL;
  MPI_Bcast(dim, 2, MPI_INT, 0, MPI_COMM_WORLD);

  int size1;
  MPI_Type_size(MPI_FLOAT,&size1);
  //printf("%d\n", size1);
  int pack_size = 64*1024/size1; //communicating data  in chunks 64KB (64*1024/8) in row
 
  // Create the datatype
  MPI_Datatype float_type;

  MPI_Type_contiguous(pack_size, MPI_FLOAT, &float_type); 
  MPI_Type_commit(&float_type);

  float global_min, *local_min, *gather_local_min, *final_gather;
  int row_end;
  
  double time0;


  if (!rank)
  {
  
    //COMMUNICATION
    row_end = ROWS/size + (ROWS - size*(ROWS/size)); //taking extra rows for computation that don't divide evenly by size
    //int s = (size-1)*(ROWS/size)*(COL/pack_size) ;
    long long int total_sends = (COL/pack_size);
    MPI_Request request1[(size-1)*(ROWS/size)];
    MPI_Status status1[(size-1)*(ROWS/size)];


    //sending part of data array to other processes
    int k = 0;
    for(int p = 1; p<size; p++)
    {
      for(int i = 0; i<(ROWS/size); i++)
     {
        MPI_Isend(&arr[row_end + ((p-1)*(ROWS/size)) + i][0], total_sends, float_type, p, p, MPI_COMM_WORLD,&request1[k++]); // rank 0 sending data to 1 
        //MPI_Send(&arr[row_end + ((p-1)*(ROWS/size)) + i][0], total_sends, float_type, p, p, MPI_COMM_WORLD);
     }
     // MPI_Isend(&arr[row_end + ((p-1)*(ROWS/size))][0], total_sends, float_type, p, p, MPI_COMM_WORLD,&request1[p-1]); // rank 0 sending data to 1 

    }
    
     ///COMPUTATION
     //float stime0 = MPI_Wtime();
     local_min = malloc(ROWS*sizeof(float));

     //computing all rows pertaining to row 0
     for (int i=0;i<row_end;i++)
     {
        float minm = FLT_MAX;
        for (int j=0;j<COL;j++)
        {
          //printf("%d %d\n",i,j);
          if (arr[i][j]<minm) minm = arr[i][j];
        }
        local_min[i] = minm;
     }
     //computing all rows that are to be computed by other processes but left after pack_size multiples

     for (int i=row_end; i<ROWS;i++)
     {
      float minm = FLT_MAX;
      for (int j= (COL/pack_size)*pack_size; j<COL; j++)      
      {
        if (arr[i][j]<minm) minm = arr[i][j];
      }
      local_min[i] = minm;
     }

     //float etime0 = MPI_Wtime();
     //time0 = etime0-stime0;

     //Wait for all Isends to other process to complete
     MPI_Waitall((size-1)*(ROWS/size),request1,status1);


     gather_local_min = malloc(ROWS*sizeof(float));
   
     
  }



  if (rank)
  {
    ROWS = dim[0], COL = dim[1];

    long long int total_recvs = (COL/pack_size);
    MPI_Request request[(ROWS/size)];
    MPI_Status status[(ROWS/size)];

    float *b = malloc((ROWS/size) * COL * sizeof(float));
    float** temp = malloc((ROWS/size) * sizeof(float*));
    for(int i = 0; i < (ROWS/size); i++) 
      temp[i] = b + COL*i;

    for (int i=0;i<(ROWS/size);i++)
    {
      MPI_Irecv(&temp[i][0], total_recvs , float_type, 0, rank, MPI_COMM_WORLD, &request[i]); // receiving data sent from root
      //MPI_Recv(&temp[i][0], total_recvs , float_type, 0, rank, MPI_COMM_WORLD, &status[i]);
    }

    MPI_Waitall((ROWS/size), request, status);



     //COMPUTATION

     local_min = malloc((ROWS/size) *sizeof(float));

     for (int i=0;i<(ROWS/size);i++)
     {
        float minm = FLT_MAX;
        for (int j=0;j<COL;j++)
        {
          if (temp[i][j]<minm) minm = temp[i][j];
        }
        local_min[i] = minm;
     }   

  }


  //int to_send = to_recv;
  //if (rank!=0 || rank!=(size-1)) to_send=to_recv/2;

  MPI_Gather(local_min, (ROWS/size), MPI_FLOAT, gather_local_min, (ROWS/size), MPI_FLOAT, 0, MPI_COMM_WORLD);
  
  

    
  if(!rank)
  {

    final_gather = malloc(ROWS*sizeof(float));
    for (int i=0;i<row_end;i++) final_gather[i] = local_min[i];

    int extra = row_end-(ROWS/size);

    //For all rows that were sent out, we still have to check whether their minimum lies in the ..
    //value that was returned or the value that was computed by root for elements that weren't sent to other processes.

    for (int i=row_end; i<ROWS;i++)
    {
      if (local_min[i]<gather_local_min[i-extra]) final_gather[i] = local_min[i]; //since this is root's local_min arr it is..
      //..of size ROWS and not (ROWS/size)
      else final_gather[i] = gather_local_min[i-extra];

    }
    

    global_min = FLT_MAX;
    for (int i=0;i<ROWS;i++)
    {
      if (global_min>final_gather[i]) global_min = final_gather[i];
      //printf("%d %lf \n", i, final_gather[i]);
    }
    //printf("\n");
    printf("%0.2f\n" , global_min);
  }

  double etime = MPI_Wtime();

  double time = etime - stime;

  double maxtime;
  MPI_Reduce (&time, &maxtime, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);


  if(!rank) 
    {
      //printf("The time taken for rank 0 computation is : %lf\n", time0);
      fptr = fopen("output.txt", "w");
      for (int i=0;i<ROWS;i++)
      {
      	if(i==ROWS-1) fprintf(fptr, "%0.2f\n", final_gather[i]);
      	else fprintf(fptr, "%0.2f,", final_gather[i]);
      }
      fprintf(fptr, "%0.2f\n", global_min);
      fprintf(fptr, "%lf\n", maxtime);
      printf("The total time taken is : %lf\n", maxtime);
    }

   

    
  MPI_Finalize();
  return 0;
}
