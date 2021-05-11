#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <assert.h>
#include "mpi.h"

FILE* fptr_b, *fptr_r, *fptr_g, *fptr_a; 

int topo[6][17] = {
{2,3,4,5,6,7,8,9,10,11,12,14,15,16,31,-1,-1},
{13,17,18,19,20,21,22,23,24,25,26,27,28,29,30,32,-1},
{33,34,35,36,37,38,39,40,41,42,43,44,46,-1,-1,-1,-1},
{45,47,48,49,50,51,52,53,54,56,58,59,60,61,-1,-1,-1},
{62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78},
{79,80,81,82,83,84,85,86,87,88,89,90,91,92,-1,-1,-1}
};
char hostname[100];
int len;
int node_num;
int group_num =-1;

int newrank=-1,newsize,leader=-2,curr_leader; // global variables for intranode communication
int newrank_intra =-1,newsize_intra,leader_intra=-2,curr_leader_intra; // global variables for intragroup communication
int newrank_inter=-1,newsize_inter,leader_inter=-2,curr_leader_inter; //global variables for internode communication.
int color = 0;
 
MPI_Comm newcomm; // intra node communicator
MPI_Comm newcomm_intra; // intra group communicator
MPI_Comm newcomm_inter; //inter group communicator

void opt_gather_nb(double* arr,int rank, int PPN,int root,int N,int size)
{

  double *rbuf = (double *) malloc(N*size*sizeof(double));

  MPI_Request request;
  MPI_Status status;

  MPI_Request request1[size];
  MPI_Status status1[size];

  if(rank!=root)
  { 
    MPI_Isend(arr, N, MPI_DOUBLE, root, 99, MPI_COMM_WORLD, &request); // all the other process sending data to root 
    MPI_Wait(&request,&status);
  }
  else
  { // if the process is root
    for(int i =0 ; i<size; i++)
    {
      if(i!=root)
      {
      int temp = i;
      if(temp>root) temp--;
      MPI_Irecv(rbuf+(i*N), N , MPI_DOUBLE, i, 99, MPI_COMM_WORLD, &request1[temp]); // root is receiving data from all the other processes

      }

      else 
      { 
        for(int i = 0 ; i < N; i++ )
        {
        rbuf[(root*N)+i] = arr[i];
        }
      }
    }
    MPI_Waitall(size-1,request1,status1);

  } 
  
}






//function for optimized gather 
void opt_gather_comm(double* arr,int rank, int PPN,int root,int N,int group_num)
{

  // for PPN > 1 , for intranode communication, one rank on the same node can collect data from all the ranks on the same node
  if(PPN>1)
  { 
  /**********************************************INTRANODE COMMUNICATION GATHER******************************************/
  double *recvbuf = malloc(PPN* N * sizeof(double));  
  
  // leader on each node is gathering data from all the other processes
  MPI_Gather(arr,N,MPI_DOUBLE,recvbuf,N,MPI_DOUBLE,leader,newcomm);  

 
 /******************************************************INTRAGROUP GATHER ************************************************/


  double* recvbuf_intra = malloc(PPN* N *newsize_intra* sizeof(double));  
 
  if(newrank==leader)
   {
	   leader_intra = curr_leader_intra;
	  
     // leader_intra in each group is gathering data from all the other processes
	   MPI_Gather(recvbuf,N*PPN,MPI_DOUBLE,recvbuf_intra,N*PPN,MPI_DOUBLE,leader_intra,newcomm_intra); 
    }
   
/******************************************************INTERGROUP GATHER ************************************************/  

  double *recvbuf_inter = malloc(PPN * N * newsize_intra * newsize_inter * sizeof(double));
  
  if (newrank_intra == leader_intra)
   { 
    leader_inter = curr_leader_inter;
	 
	  // leader_inter in one group is gathering data from all the other processes
	  MPI_Gather(recvbuf_intra,N*PPN*newsize_intra,MPI_DOUBLE,recvbuf_inter,N*PPN*newsize_intra,MPI_DOUBLE,leader_inter,newcomm_inter); 

	 //debugging
    /*if (rank == root)
	  {
	  	for (int i=0;i<PPN * N * newsize_intra * newsize_inter; i++)
	  	{
	  		printf("%lf ", recvbuf_inter[i]);
	  	}
	  }*/
  } 
  

}


if (PPN == 1)
{

	/******************************************************INTRAGROUP GATHER ************************************************/

  double*  recvbuf_intra = malloc(PPN* N *newsize_intra* sizeof(double)); 
 
    

  MPI_Comm_rank(newcomm_intra, &newrank_intra);
  MPI_Comm_size(newcomm_intra, &newsize_intra);

  // leader_intra in each group is gathering data from all the other processes
  MPI_Gather(arr,N*PPN,MPI_DOUBLE,recvbuf_intra,N*PPN,MPI_DOUBLE,leader_intra,newcomm_intra); 



  /******************************************************INTERGROUP GATHER ************************************************/  


  if (newrank_intra == leader_intra)
  {
  	  MPI_Comm_rank(newcomm_inter, &newrank_inter);
	    MPI_Comm_size(newcomm_inter, &newsize_inter);

    

	  double* recvbuf_inter = malloc(PPN * N * newsize_intra * newsize_inter * sizeof(double));

    leader_inter = curr_leader_inter;
	 
	  // leader_inter in one group is gathering data from all the other processes
	  MPI_Gather(recvbuf_intra,N*PPN*newsize_intra,MPI_DOUBLE,recvbuf_inter,N*PPN*newsize_intra,MPI_DOUBLE,leader_inter,newcomm_inter); 

	 //debugging
    /*if (rank == root)
	  {
	  	for (int i=0;i<PPN * N * newsize_intra * newsize_inter; i++)
	  	{
	  		printf("%lf ", recvbuf_inter[i]);
	  	}
	  }*/
  } 
    
}

}

//function for optimized MPI_Bcast() using communicator
void opt_bcast_comm(double* arr,int rank, int PPN,int root,int N,int group_num)
{
  
  // for PPN > 1 , for intranode communication, one rank on the same node can collect data from all the ranks on the same node
  if(PPN>1)
  { 
    /********************************************* InterGroup Bcast*******************************************/ 
   if (newrank_intra == leader_intra)
   { 
    leader_inter = curr_leader_inter;
	 
	  // leader of all groups (root node here) is broadcasting the data to the members of other group.
	  MPI_Bcast(arr,N,MPI_DOUBLE,leader_inter,newcomm_inter); 

 	 }
 
  /********************************************* IntraGroup Bcast*******************************************/ 
  if(newrank==leader)
   {
	   leader_intra = curr_leader_intra;
	   
     // Leaders of each group are broadcasting the data to all its group members
     MPI_Bcast(arr,N,MPI_DOUBLE,leader_intra,newcomm_intra);
    }
  
  /********************************************* IntraNode Bcast*******************************************/ 
  // leader on each node is broadcasting the data to other processes on the same node
     MPI_Bcast(arr,N,MPI_DOUBLE,leader,newcomm);

     //Debugging 
     /*if(rank == 20)
    { 
       for(int i =0; i<N;i++)
       {
        printf(" %lf ",arr[i]);
       }
      printf("\n");
    }*/
}
///****************************************BCAST FOR PPN = 1******************************************************//

if (PPN == 1)
{
/********************************************* InterGroup Bcast*******************************************/ 
  if (newrank_intra == leader_intra)
  { 
    leader_inter = curr_leader_inter;
	 
    // leader of all groups (root node here) is broadcasting the data to the members of other group.
	  MPI_Bcast(arr,N,MPI_DOUBLE,leader_inter,newcomm_inter); 
  } 
 
 /********************************************* IntraGroup Bcast*******************************************/ 

  // Leaders of each group are broadcasting the data to all its group members
   MPI_Bcast(arr,N,MPI_DOUBLE,leader_intra,newcomm_intra);
  

   //Debugging 
    /* if(rank == 13)
    { 
       for(int i =0; i<N;i++)
       {
        printf(" %lf ",arr[i]);
       }
      printf("\n");
    }*/

}
 
}

//function for new reduce 
 void opt_reduce_datatype(double* arr,int rank,int PPN,int root,int N,int size)
 {
   double *rbuf = malloc(N*sizeof(double));  
   
  int s = N/2048;
  MPI_Request request1[s];
  MPI_Status status1[s];
  MPI_Request request2[(N/2048)];
  MPI_Status status2[(N/2048)];

  // Create the datatype
  MPI_Datatype double_type;
  MPI_Type_contiguous(2048, MPI_DOUBLE, &double_type); //sending data  in chunks of 16KB
  MPI_Type_commit(&double_type);
  
  
  if(rank!=root)
    { 
      for(int i = 0; i<N/2048; i++)
       { 
         MPI_Isend(arr+(i*N/2048), 1, double_type, root, 99+i, MPI_COMM_WORLD,&request1[i]); // all the other process sending data to root 
       }
       MPI_Waitall(N/2048,request1,status1);
       
    }
  else
  {  // if the process is not root
     for(int i =0 ; i<size; i++)
     {
       if(i!=root)
       {
        //int temp = i;
        //if(temp > root) temp--;
        
        for(int j = 0; j< N/2048; j++)
         {
            MPI_Irecv(rbuf+j*(N/2048), 1 , double_type, i, 99+j, MPI_COMM_WORLD, &request2[j]); // root is receiving data from all the other processes
         } 
         MPI_Waitall(N/2048,request2,status2);
         //printf("%d\n",rank);
        
        for(int i = 0 ; i<N;i++ )
        {
          if(arr[i]<rbuf[i]) arr[i] = rbuf[i];  // performing the operation 
        }

       }
     }
     
  }
}


//function for bcast using Isend and datatype
void opt_bcast_datatype(double* arr,int rank,int PPN,int root,int N,int size)
{
  int s =  size*(N/2048);
  MPI_Request request1[s];
  MPI_Status status1[s];
  MPI_Request request2[(N/2048)];
  MPI_Status status2[(N/2048)];
 

  // Create the datatype
  MPI_Datatype double_type;
  MPI_Type_contiguous(2048, MPI_DOUBLE, &double_type); //sending data  in chunks of 16KB
  MPI_Type_commit(&double_type);
  
   if(rank!=root)
   {
    
    for(int i = 0; i<N/2048; i++)
     {
       MPI_Irecv(arr+(i*(N/2048)), 1, double_type, root, 99+i, MPI_COMM_WORLD, &request2[i]); // all the other process sending data to root 
     }
     MPI_Waitall(N/2048,request2,status2); // waiting for all the receive requests to complete

   }

  else
  {  // if the process is root
     for(int i =0 ; i<size; i++)
     {
       if(i!=root)
       {
        int temp = i;
        if(temp > root) temp--;
        {
           for(int j = 0; j < N/2048; j++ )
             MPI_Isend(arr+(j*(N/2048)), 1 , double_type, i, 99+j, MPI_COMM_WORLD,&request1[temp*(N/2048)+j]); // root is sending data to all the other processes

        }
       }
     }
     MPI_Waitall((size-1)*(N/2048),request1,status1); // waiting for all the receive requests to complete

  }

}


//function for gather using datatype
void opt_gather_datatype(double* arr,int rank,int PPN,int root,int N,int size)
{
  
  double *rbuf = (double *) malloc(N*size*sizeof(double));

  // Create the datatype
  MPI_Datatype double_type;
  MPI_Type_contiguous(2048, MPI_DOUBLE, &double_type); //sending data  in chunks of 16KB
  MPI_Type_commit(&double_type);
  
  MPI_Request request[size*(N/2048)];
  MPI_Status status[size*(N/2048)];

  MPI_Request request1[N/2048];
  MPI_Status status1[N/2048];

  if(rank!=root) // if process is not root then send data to the root process
  { 
    for(int i = 0; i < N/2048; i++ )
    {
      MPI_Isend(arr+(i*(N/2048)), 1, double_type, root, 99+i, MPI_COMM_WORLD,&request1[i]); // all the other process sending data to root 
    }
    MPI_Waitall(N/2048,request1,status1);
  }
  else
  { // if the process is root then receive data from the other processes
    for(int i =0 ; i<size; i++)
    {
      if(i!=root)
      {
      int temp = i;
      if(temp > root) temp--;
      for(int j = 0; j < N/2048 ; j++ )
       {
          MPI_Irecv(rbuf+(i*N/2048)+(j*(N/2048)), 1 , double_type, i, 99+j, MPI_COMM_WORLD, &request[temp*(N/2048)+j]); // root is receiving data from all the other processes

          //MPI_Wait(&request,&status);
       }
     
      }
      else 
      { 
        for(int i = 0 ; i < N; i++ )
        {
        rbuf[(root*N)+i] = arr[i];
        }
      }
    }
    MPI_Waitall((size-1)*N/2048,request,status);

  } 

  /*if(rank==root)
  {
    for(int i = 0; i < N*size; i++)
     printf("opt gather %d %lf \n ", i , rbuf[i]);
  }*/

}



//function for bcast using Isend
void opt_bcast_nb(double* arr,int rank,int PPN,int root,int N,int size)
{
  MPI_Request request[size];
  MPI_Status status[size];
  
  MPI_Request request1;
  MPI_Status status1;
  
   if(rank!=root)
   {
    MPI_Irecv(arr, N, MPI_DOUBLE, root, 99, MPI_COMM_WORLD, &request1); // all the other process sending data to root 
    MPI_Wait(&request1,&status1);
   }

  else
  {  // if the process is root
     for(int i =0 ; i<size; i++)
     {
       if(i!=root)
       {
        int temp = i;
        if(temp > root) temp--;
        MPI_Isend(arr, N , MPI_DOUBLE, i, 99, MPI_COMM_WORLD,&request[temp]); // root is sending data to all the other processes

       }
     }
     MPI_Waitall(size-1,request,status);
  }

}


//function for new reduce 
 void opt_reduce_nb(double* arr,int rank,int PPN,int root,int N,int size)
 {
   double *rbuf = malloc(N*sizeof(double));  
   MPI_Request request;
   MPI_Status status;
   
   MPI_Request request1;
   MPI_Status status1;
  
   if(rank!=root)
    { MPI_Isend(arr, N, MPI_DOUBLE, root, 99, MPI_COMM_WORLD,&request); // all the other process sending data to root 
      MPI_Wait(&request,&status);
    }
  else
  {  // if the process is not root
     for(int i =0 ; i<size; i++)
     {
       if(i!=root)
       {
        int temp = i;
        if(temp > root) temp--;
        MPI_Irecv(rbuf, N , MPI_DOUBLE, i, 99, MPI_COMM_WORLD, &request1); // root is receiving data from all the other processes
        MPI_Wait(&request1,&status1);
        
        for(int i = 0 ; i<N;i++ )
        {
          if(arr[i]<rbuf[i]) arr[i] = rbuf[i];  // performing the operation 
        }

       }
     }
    
  }
   
}



//function for creating communicator if PPN is equal to one
void create_comm_o(int rank,int root, int PPN)
{
  /******************************************************INTRAGROUP Communicator Creation ************************************************/

  MPI_Comm_split(MPI_COMM_WORLD,group_num,rank,&newcomm_intra);    

  MPI_Comm_rank(newcomm_intra, &newrank_intra);
  MPI_Comm_size(newcomm_intra, &newsize_intra);

  MPI_Status status;

  if(rank==root)
  {
    curr_leader_intra = newrank_intra;   // newrank of the leader of each group in newcomm_intra (in its group)
  }

 // all process in COMM_WORLD will have same leader_intra number as newrank_intra of root in newcomm_intra
  MPI_Bcast(&curr_leader_intra,1,MPI_INT,root,MPI_COMM_WORLD); 
  leader_intra = curr_leader_intra;
  
  /******************************************************INTERGROUP Communicator Preprocessing************************************************/  
  if(newrank_intra!=leader_intra)
      color = MPI_UNDEFINED;


  MPI_Comm_split(MPI_COMM_WORLD,color,rank,&newcomm_inter); 
 
  if (newrank_intra == leader_intra)
  {
  	  MPI_Comm_rank(newcomm_inter, &newrank_inter);
	    MPI_Comm_size(newcomm_inter, &newsize_inter);

    
   if(rank==root)
	  {
	    curr_leader_inter = newrank_inter;   // newrank of the leader of all groups in newcomm_inter
	    
      for(int i = 0 ; i < newsize_inter; i++ )
       { 
         //printf("Hello %d %d \n",i,newrank_inter);
         if(i!=newrank_inter)
          {
           //printf("rank : %d i :%d\n",rank,i);
           MPI_Send(&curr_leader_inter,1,MPI_INT,i,99,newcomm_inter); // broadcasting newrank of leader in to leaders of other group.
          }
       } 
	   
    }
    else
    {
      //printf("rank : %d newrank_inter :%d\n",rank,newrank_inter);
      MPI_Recv(&curr_leader_inter,1,MPI_INT,MPI_ANY_SOURCE,99,newcomm_inter,&status);
    }
  }  
	 
    // all process in COMM_WORLD will have same leader_inter number as newrank_inter of root in newcomm_inter
	  //MPI_Bcast(&curr_leader_inter,1,MPI_INT,root,MPI_COMM_WORLD); 
    
}

//function for creating communicator if PPN is greater than one
void create_comm_g(int rank, int root, int PPN)
{
   /**********************************************INTRANODE COMMUNICATOR Preprocessing******************************************/
  
  MPI_Comm_split(MPI_COMM_WORLD,node_num,rank,&newcomm);
  
  MPI_Comm_rank(newcomm, &newrank);
  MPI_Comm_size(newcomm, &newsize);
   
  leader = root%PPN;

 /******************************************************INTRAGROUP COMMUNICATOR Preprocessing ************************************************/

 
  if(newrank!=leader)
     group_num = MPI_UNDEFINED;
    
  MPI_Comm_split(MPI_COMM_WORLD,group_num,rank,&newcomm_intra);    

  if(newrank == leader)
  {
    MPI_Comm_rank(newcomm_intra, &newrank_intra);
	  MPI_Comm_size(newcomm_intra, &newsize_intra);

	  if(rank==root)
	  {
	    curr_leader_intra = newrank_intra;   // newrank of the leader of each group in newcomm_intra (in its group)
	  }

  }
	  // all process in COMM_WORLD will have same leader_intra number as newrank_intra of root in newcomm_intra
	  MPI_Bcast(&curr_leader_intra,1,MPI_INT,root,MPI_COMM_WORLD); 
     
     
   
   if(newrank==leader)
   {
	   leader_intra = curr_leader_intra;
	 
   }
   
 /******************************************************INTERGROUP COMMUNICATOR Preprocessing ************************************************/  

  
  if(newrank_intra!=leader_intra)
      color = MPI_UNDEFINED;


  MPI_Comm_split(MPI_COMM_WORLD,color,rank,&newcomm_inter); 
 
  if (newrank_intra == leader_intra)
  {
  	MPI_Comm_rank(newcomm_inter, &newrank_inter);
	  MPI_Comm_size(newcomm_inter, &newsize_inter);
    MPI_Status status;
    
    if(rank==root)
	  {
	    curr_leader_inter = newrank_inter;   // newrank of the leader of all groups in newcomm_inter
	    
      for(int i = 0 ; i < newsize_inter; i++ )
       { 
         //printf("Hello %d %d \n",i,newrank_inter);
         if(i!=newrank_inter)
          {
           //printf("rank : %d i :%d\n",rank,i);
           MPI_Send(&curr_leader_inter,1,MPI_INT,i,99,newcomm_inter); // broadcasting rank of leaders in inter group communication to other group leaders
          }
       }
    }
    else
    {
      //printf("rank : %d newrank_inter :%d\n",rank,newrank_inter);
      MPI_Recv(&curr_leader_inter,1,MPI_INT,MPI_ANY_SOURCE,99,newcomm_inter,&status);
    }
  }  

    // all process in COMM_WORLD will have same leader_inter number as newrank_inter of root in newcomm_inter
	  //MPI_Bcast(&curr_leader_inter,1,MPI_INT,root,MPI_COMM_WORLD);
 
  
}


//function for optimized MPI_Reduce()
void opt_reduce_comm(double* arr,int rank, int PPN,int root,int N,int group_num)
{
  // for PPN > 1 , for intranode communication, one rank on the same node can collect data from all the ranks on the same node
  if(PPN>1)
  { 
  /**********************************************INTRANODE COMMUNICATION Reduce******************************************/
 
  
  double *recvbuf = malloc(N*sizeof(double));  
 
 // leader on each node is reducing data from all the other processes
  MPI_Reduce(arr,recvbuf, N,MPI_DOUBLE,MPI_MAX,leader,newcomm);  

 /******************************************************INTRAGROUP Reduce ************************************************/
  double* recvbuf_intra = malloc(N * sizeof(double));  

	if(newrank==leader)
   {
	   leader_intra = curr_leader_intra;
	  
     // leader_intra in each group is gathering data from all the other processes
	   MPI_Reduce(recvbuf,recvbuf_intra,N,MPI_DOUBLE,MPI_MAX,leader_intra,newcomm_intra); 
    }
   
 
  /******************************************************INTERGROUP Reduce************************************************/  
  double *recvbuf_inter = malloc(N *sizeof(double));

  if (newrank_intra == leader_intra)
  { 
    leader_inter = curr_leader_inter;
	 
	  // leader_inter in one group is getting reduce data from all the other processes in the group
	  MPI_Reduce(recvbuf_intra,recvbuf_inter,N,MPI_DOUBLE,MPI_MAX,leader_inter,newcomm_inter); 

	 //debugging
    /*if (rank == root)
	  {
	  	for (int i=0;i< N ; i++)
	  	{
	  		printf("%lf ", recvbuf_inter[i]);
	  	}
      printf("\n");
	  }*/
  } 
}
  if (PPN == 1)
 {

	/******************************************************INTRAGROUP Reduce ************************************************/
  

  MPI_Comm_rank(newcomm_intra, &newrank_intra);
  MPI_Comm_size(newcomm_intra, &newsize_intra);

  double* recvbuf_intra = malloc(N * sizeof(double));  

  leader_intra = curr_leader_intra;
  
 // leader_intra in each group is receiving the reduce data from all the other processes in its group
  MPI_Reduce(arr,recvbuf_intra,N,MPI_DOUBLE,MPI_MAX,leader_intra,newcomm_intra); 



  /******************************************************INTERGROUP Reduce ************************************************/  
  
 
  if (newrank_intra == leader_intra)
  {
  	MPI_Comm_rank(newcomm_inter, &newrank_inter);
	  MPI_Comm_size(newcomm_inter, &newsize_inter);

    double* recvbuf_inter = malloc(N * sizeof(double));

    leader_inter = curr_leader_inter;
	 
	  // reduced data on root
	  MPI_Reduce(recvbuf_intra,recvbuf_inter,N,MPI_DOUBLE,MPI_MAX,leader_inter,newcomm_inter); 

	 //debugging
   /* if (rank == root)
	  {
	  	for (int i=0;i<N; i++)
	  	{
	  		printf("%lf ", recvbuf_inter[i]);
	  	}
      printf("\n");
	  }*/
  } 
    
}
}

// main 
int main(int argc, char* argv[])
{
	//Initializations
	MPI_Init(&argc, &argv);
	int root = atoi(argv[1]); //root in case of Bcast, Reduce and Gather
  int D = atoi(argv[2]); //number of doubles
  int PPN = atoi(argv[3]); //number of processes per node (1 or 8)
  int N = D*1024/8;
  //int N = 4;  // for debugging

  double stime,etime,time,maxTime,comm_time,opt_maxTime;
  


	int rank, size;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

  int nodes = size/PPN;  //number of total nodes (4 or 16)
  MPI_Get_processor_name(hostname, &len);


  //for finding the node number(processor number) of process 
  if (len==6) node_num = hostname[5]-'0';
  else node_num = (hostname[5]-'0')*10 + (hostname[6]-'0');
  
  ///For finding Group Number
  for (int i=0;i<6;i++)
   {
    for (int j=0;j<17;j++)
    {
      if (node_num==topo[i][j])
      {
        group_num = i; // this process belongs to group (i+1)
        break;
      }
    }
  }


  // for opening files 
	if (rank == root)
	{
		char file_prefix[100];
		snprintf(file_prefix, 40, "data_Bcast.txt");
		fptr_b = fopen(file_prefix, "a");

    snprintf(file_prefix, 40, "data_Reduce.txt");
    fptr_r = fopen(file_prefix, "a");

    snprintf(file_prefix, 40, "data_Gather.txt");
    fptr_g = fopen(file_prefix, "a");

    snprintf(file_prefix, 40, "data_Alltoallv.txt");
    fptr_a = fopen(file_prefix, "a");
	}
  
  

	
  

                                             //// COLLECTIVE CALL - BCAST  /////


  double *arr = malloc(N * sizeof(double));
  for(int i = 0; i < N; i++) 
  {
    //arr[i] = rank+i; //for debugging
      arr[i] = (double)rand(); //actual
  }

  //standard
  stime = MPI_Wtime();
  for (int i=0;i<5;i++) 
  {
   MPI_Bcast(arr, N, MPI_DOUBLE, root, MPI_COMM_WORLD);
  }
  etime = MPI_Wtime();
  time = etime-stime;
    
  MPI_Reduce (&time, &maxTime, 1, MPI_DOUBLE, MPI_MAX, root, MPI_COMM_WORLD);
  if (rank==root) 
  {
    printf("\n");
    printf("Average time taken for standard Bcast : %lf\n", maxTime/5);
    fprintf (fptr_b, "%d %d %d %d %lf\n",D,size/PPN, PPN, 0, maxTime/5);
  }
 
  for(int i = 0; i < N; i++) 
  {
    //arr[i] = rank+i; //for debugging
    arr[i] = (double)(rand())/RAND_MAX; //actual
  }

  //optimized Bcast
  stime = MPI_Wtime();
  for(int i = 0; i<5; i++){
     
  if (D==16) opt_bcast_nb(arr,rank,PPN,root,N,size);
  
  else if (D==256 || (D==2048 && (size == 4||size==16||size==32))) opt_bcast_datatype(arr,rank,PPN,root,N,size);

  else if (D==2048 && size ==128)
    {
      if(PPN==1) create_comm_o(rank,root,PPN);
        
      if(PPN>1) create_comm_g(rank,root,PPN);
      
      opt_bcast_comm(arr,rank,PPN,root,N,group_num);
    }
  }
  
  etime = MPI_Wtime();
  time = etime-stime;
  MPI_Reduce (&time, &opt_maxTime, 1, MPI_DOUBLE, MPI_MAX, root, MPI_COMM_WORLD);
  if (rank==root) 
  {
   printf("Average time taken for optimized Bcast %lf\n", opt_maxTime/5);
   printf("Speedup of std Bcast and opt Bcast : %lf\n", maxTime/opt_maxTime);
   printf("\n");
   
   fprintf (fptr_b, "%d %d %d %d %lf\n",D,size/PPN, PPN, 1, opt_maxTime/5);
  }

  
 MPI_Barrier(MPI_COMM_WORLD);


                                        //// COLLECTIVE CALL - REDUCE  /////
  
  for(int i = 0; i < N; i++) 
  {
    //arr[i] = rank+i; //for debugging
    arr[i] = (double)(rand())/RAND_MAX; //actual
  }
  
   
  //standard
  stime = MPI_Wtime();
  for (int i=0;i<5;i++)  // change it to 5 later
  {
    double* buffer;
    buffer = (double *) malloc(N*sizeof(double));
    MPI_Reduce (arr, buffer, N, MPI_DOUBLE, MPI_MAX, root, MPI_COMM_WORLD);
  }  
  etime = MPI_Wtime();
  time = etime-stime;
  MPI_Reduce (&time, &maxTime, 1, MPI_DOUBLE, MPI_MAX, root, MPI_COMM_WORLD);
  if (rank==root) 
  {
    printf("Average time taken for standard Reduce : %lf\n", maxTime/5);
    fprintf (fptr_r, "%d %d %d %d %lf\n",D,size/PPN, PPN, 0, maxTime/5);
  }

  //Debugging
  /*if(rank==root)
  {
     for(int i = 0; i < N; i++)
      {
        printf("standard reduce %lf",buffer[i] );
      }
  }*/

  
  for(int i = 0; i < N; i++) 
  {
    //arr[i] = rank+i; //for debugging
    arr[i] = (double)(rand())/RAND_MAX; //actual
  }
  
  //optimized
  stime = MPI_Wtime();
  for(int i=0; i < 5 ; i++){ 


   if(D==16) opt_reduce_nb(arr,rank,PPN,root,N,size);

   
   else if (D==256 || (D==2048 && (size == 4||size==16||size==32))) opt_reduce_datatype(arr,rank,PPN,root,N,size);


   else if (D==2048 && size == 128)
    
    {  if(PPN==1) create_comm_o(rank,root,PPN);
       
       if(PPN>1) create_comm_g(rank,root,PPN);
       
       opt_reduce_comm(arr,rank,PPN,root,N,group_num);
    } 
  }
  etime = MPI_Wtime();
  time = etime-stime;
  
  MPI_Reduce (&time, &opt_maxTime, 1, MPI_DOUBLE, MPI_MAX, root, MPI_COMM_WORLD);
    
  if (rank==root) 
  {
   printf("Average time taken for optimized Reduce : %lf\n", opt_maxTime/5);
   printf("Speedup of std reduce and opt reduce : %lf\n", maxTime/opt_maxTime);
   printf("\n");
   
   fprintf (fptr_r, "%d %d %d %d %lf\n",D,size/PPN, PPN, 1, opt_maxTime/5);
  }

  /*if(rank==root)
  {
     for(int i = 0; i < N; i++)
      {
        printf("optimized reduce %lf",arr[i] );
      }
  }*/
   


 MPI_Barrier(MPI_COMM_WORLD);


                                        //// COLLECTIVE CALL - GATHER  /////

  for(int i = 0; i < N; i++) 
  {
    //arr[i] = rank+i; //for debugging
    arr[i] = (double)(rand())/RAND_MAX; //actual
  }
   
  
    
  //standard
  stime = MPI_Wtime();
  for (int i=0;i<5;i++)  // need to change to 5 later
  {
   double* buffer_gather;
   buffer_gather = (double *) malloc(size*N*sizeof(double));
   
   MPI_Gather (arr, N, MPI_DOUBLE, buffer_gather, N, MPI_DOUBLE, root, MPI_COMM_WORLD);
  }
  etime = MPI_Wtime();
  time = etime-stime;

  MPI_Reduce (&time, &maxTime, 1, MPI_DOUBLE, MPI_MAX, root, MPI_COMM_WORLD);
  if (rank==root) 
  {
    printf("Average time taken for standard Gather : %lf\n", maxTime/5);
    fprintf (fptr_g, "%d %d %d %d %lf\n",D,size/PPN, PPN, 0, maxTime/5);
  }


  for(int i = 0; i < N; i++) 
  {
    //arr[i] = rank+i; //for debugging
    arr[i] = (double)(rand())/RAND_MAX; //actual
  }
   
  

  //optimized
  stime = MPI_Wtime();
  for(int i = 0 ; i < 5 ; i++ ){

   if(D==16)  opt_gather_nb(arr,rank,PPN,root,N,size); 
  
   else if (D==256 || (D==2048 && (size == 4||size==16||size==32))) opt_gather_datatype(arr,rank,PPN,root,N,size);
   
   else if (D==2048 && size ==128) 
    {
         
    if(PPN==1) create_comm_o(rank,root,PPN);
       
    if(PPN>1) create_comm_g(rank,root,PPN);
    
    opt_gather_comm(arr,rank,PPN,root,N,group_num);
    }
  } 
    etime = MPI_Wtime();
    time = etime-stime;

    MPI_Reduce (&time, &opt_maxTime, 1, MPI_DOUBLE, MPI_MAX, root, MPI_COMM_WORLD);
   

  if (rank==root) 
  {
   printf("Average time taken for optimized Gather : %lf\n", opt_maxTime/5);
   printf("Speedup of std Gather and opt Gather : %lf\n", maxTime/opt_maxTime);
   printf("\n");
   
   fprintf (fptr_g, "%d %d %d %d %lf\n",D,size/PPN, PPN, 1, opt_maxTime/5);
  }

  MPI_Barrier(MPI_COMM_WORLD);



                                   //// COLLECTIVE CALL - ALLTOALLV  /////
  
/************************Preprocessing for ALLTOALLV********************************************/
  
  double *sbuf, *rbuf;
  int *sendcounts, *recvcounts, *rdispls, *sdispls;
  int i, j;

  sbuf = (double *)malloc( N * sizeof(double) );
  rbuf = (double *)malloc( size * N * sizeof(double) );
  
  /* Load up the buffers */
  for (i=0; i<N; i++) {
      sbuf[i] = rand();
  }

  for (int i=0;i<N*size;i++)
  {
    rbuf[i] = -i;
  }
  
  sendcounts = (int *)malloc( size * sizeof(int) );
  recvcounts = (int *)malloc( size * sizeof(int) );
  rdispls = (int *)malloc( size * sizeof(int) );
  sdispls = (int *)malloc( size * sizeof(int) );

  

  for (int i=0;i<size;i++)
    {
      int rand1 = ((int)rand())%N; 
      int rand2 = ((int)rand())%(N-rand1);
      sendcounts[i] = rand1;
      sdispls[i] = rand2;
    }

  
    
    //printf("before all to all %d \n",rank );

 
    MPI_Alltoall(sendcounts, 1, MPI_INT, recvcounts, 1, MPI_INT, MPI_COMM_WORLD); // sending sendcounts to other processes 
    //printf("after all to all %d \n",rank );

    rdispls[0] = 0;
    int sum = 0;
    for (int i=1;i<size;i++)
    {
      sum+= recvcounts[i-1];
      rdispls[i] = sum; // calculation rdispls on the basis of recvcounts
    }

/****************************************************ALLTOALLV******************************************/
  //standard
  stime = MPI_Wtime();
  for (int i=0;i<5;i++)
  {
    MPI_Alltoallv( sbuf, sendcounts, sdispls, MPI_DOUBLE,
                       rbuf, recvcounts, rdispls, MPI_DOUBLE, MPI_COMM_WORLD );

  }
  etime = MPI_Wtime();
  time = etime-stime;

  MPI_Reduce (&time, &maxTime, 1, MPI_DOUBLE, MPI_MAX, root, MPI_COMM_WORLD);

   if (rank==root) 
  {
    printf("Average time taken for standard AllToAllv : %lf\n", maxTime/5);
    fprintf (fptr_a, "%d %d %d %d %lf\n",D,size/PPN, PPN, 0, maxTime/5);
  }

  
  //optimized
  stime = MPI_Wtime();
  for(int i = 0 ; i < 5 ; i++ ){
    
    //opt_alltoallv();
    MPI_Request request1[size];
    MPI_Request request2[size];
    MPI_Status status1[size];
    MPI_Status status2[size];

    for (int i=0;i<size;i++)
    {
      //if (sendcounts[i]==0) continue;
      MPI_Isend(sbuf+sdispls[i], sendcounts[i], MPI_DOUBLE, i, 99, MPI_COMM_WORLD, &request1[i]);
    }
    for (int i=0;i<size;i++)
    {
      //if(recvcounts[i]==0) continue;
      MPI_Irecv(rbuf+rdispls[i], recvcounts[i], MPI_DOUBLE, i, 99, MPI_COMM_WORLD, &request2[i]);
    }
    MPI_Waitall(size, request1, status1); // waiting for all the send requests to complete
    MPI_Waitall(size, request2, status2); // waiting for all the receive requests to complete
    
  }
  etime = MPI_Wtime();
  time = etime-stime;

  MPI_Reduce (&time, &opt_maxTime, 1, MPI_DOUBLE, MPI_MAX, root, MPI_COMM_WORLD);
  if (rank==root) 
  {
      printf("Average time taken for optimized AlltoAllv : %lf\n", opt_maxTime/5);
      printf("Speedup of std AlltoAllv and opt AlltoAllv : %lf\n", maxTime/opt_maxTime);
      printf("\n");
    fprintf (fptr_a, "%d %d %d %d %lf\n",D,size/PPN, PPN, 1, opt_maxTime/5);
  }
  MPI_Barrier(MPI_COMM_WORLD);  
  
  MPI_Finalize();
  return 0;
}
