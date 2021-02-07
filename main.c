/* 
 * File:   main.c
 * Author: adams
 *
 * Created on October 22, 2019, 4:21 AM
 */

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>
void Get_input(
        int my_rank,
        int comm_sz,
        int* m_p,
        int* k_p);
    int m;  //the rows of matrix
    int K;  //the cols of matrix

int main(void)
{
    int i, j, k, my_rank, comm_sz, anstag;
    int source = 0,msg[K], ans;
    double starttime,endtime;
    double tmp,totaltime;
    MPI_Status status;
    MPI_Init(NULL, NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);     
    Get_input(my_rank,comm_sz,&m,&K);
    double A[m][K], B[K], C[m];
        if(my_rank == source){
            starttime = MPI_Wtime();
            for (i = 0; i < K; i++)
            {
                B[i] = rand()%10;
                for (j = 0; j < m; j++)
                {
                    A[j][i] = rand()%10;
                }
            }
            //bcast the B vector to all slave processor
            MPI_Bcast(B, K, MPI_DOUBLE, source, MPI_COMM_WORLD);
            //partition the A matrix to all slave processor
            for (i = 1; i < comm_sz; i++)
            {
                for (k = i - 1; k < m; k += comm_sz - 1)
                {
                    for (j = 0; j < k; j++)
                    {    
                        msg[j] = A[K][j];
                    }
                    MPI_Send(msg, k, MPI_DOUBLE, i, k, MPI_COMM_WORLD);
                }
            }
        }
        else{
            starttime = MPI_Wtime();
            MPI_Bcast(B, K, MPI_DOUBLE, source, MPI_COMM_WORLD);
            //every processor receive the part of A matrix,and make Mul operator with B vector
            for ( i = my_rank - 1; i < m; i += comm_sz - 1){
                MPI_Recv(msg, K, MPI_DOUBLE, source, i, MPI_COMM_WORLD, &status);
                ans = 0;
        
                for ( j = 0; j < k; j++)
                {
                    ans += msg[j] * B[j];
                }
                //send back the result
                MPI_Send(&ans, 1, MPI_DOUBLE, source, i, MPI_COMM_WORLD);
            }
            endtime = MPI_Wtime();
            tmp = endtime - starttime;
        } 
        if(my_rank == source){    
            //receive the result from all slave processor
            for ( i = 0; i < m; i++)
            {
                MPI_Recv(&ans, 1, MPI_DOUBLE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
                anstag = status.MPI_TAG;
                C[anstag] = ans;
            }
            //print the result
            for (i = 0; i < m; i++)
            {   
                printf("%lf",C[i]);
                if((i+1)%20 == 0)
                    printf("\n");
            }
        }
    
    endtime = MPI_Wtime();
    totaltime = endtime - starttime;
    printf("cost time:%f s.\n",tmp); 
    MPI_Reduce(&tmp,&totaltime,1,MPI_DOUBLE,MPI_SUM,0,MPI_COMM_WORLD);
    if(my_rank == source)
        printf("total time:%f s.\n",totaltime);
    MPI_Finalize();
    return 0;
}
void Get_input(
        int my_rank,
        int comm_sz,
        int* m_p,
        int* k_p){
    int dest;
    
    if(my_rank ==0)
    {
        printf("enter m and k\n");
        scanf("%d %d", m_p, k_p);
        for (dest = 1; dest < comm_sz; dest++)
        {
            MPI_Send(m_p,1,MPI_INT, dest,0,MPI_COMM_WORLD);
            MPI_Send(k_p,1,MPI_INT, dest,0,MPI_COMM_WORLD);
        }//for
    }else
    {
        MPI_Recv(m_p,1,MPI_INT,0,0,MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(k_p,1,MPI_INT,0,0,MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }//else
}//getinput