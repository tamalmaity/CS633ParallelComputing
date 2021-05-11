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
    
    float *a = malloc(rows* col* sizeof(float));
    arr = malloc(rows * sizeof(float*));
    

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
            arr[count-2][arr_index-2] = (float)atof(temp);
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

  float global_min, *local_min;
  
  double time0;

   ///COMPUTATION
   double stime0 = MPI_Wtime();
   local_min = malloc(col*sizeof(float));

   global_min = FLT_MAX;

   for (int i=0;i<col;i++)
   {
      float minm = FLT_MAX;
      for (int j=0;j<rows;j++)
      {
        if (arr[j][i]<minm) minm = arr[j][i];
      }
      local_min[i] = minm;

      if(global_min>local_min[i]) global_min = local_min[i];
   }
   double etime0 = MPI_Wtime();
   time0 = etime0-stime0;



  double etime = MPI_Wtime();

  double time = etime - stime;

  double maxtime;
  MPI_Reduce (&time, &maxtime, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);


  printf("The time taken for rank 0 computation is : %lf\n", time0);
  printf("The total time taken is : %lf\n", maxtime);

  FILE* fptr;
  fptr = fopen("true_res.txt", "w");
  for (int i=0;i<col;i++) fprintf(fptr, "%d %f\n", i, local_min[i]);
    fprintf(fptr,"%f", global_min);
   
    
  MPI_Finalize();
  return 0;
}
