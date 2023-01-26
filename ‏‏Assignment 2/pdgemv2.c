#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <cblas.h>

extern void dgemv_(char *transa, int *m, int *n, double *alpha, double *a, int *lda, double *b,
                   int *ldb, double *dzero, double *beta, int *ldc );

int main (int argc, char * argv[]) {

    //================= variables initialization - start =================
    int rowsNum = 128, colsNum = 128; //matrix dimension
    int i,j; //indexes for loops

    int my_rank, procsNum; //processes variables
    double start_time, end_time;

    double d_zero=0.0, d_one=1.0;
    int one=1;

    double *x, *local_x, *local_A, *y, *local_y;
    //================= variables initialization - start =================


    //================= MPI initialization - start =================
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &procsNum);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    //================= MPI initialization - end =================


    //================= block sizes - start =================
    //assumption - rows and cols divide by processes number
    int rowsNum_local = rowsNum / procsNum;
    int vsize = colsNum;
    int vsize_local = vsize / procsNum;

    if (rowsNum % procsNum != 0 || vsize % procsNum != 0) {
        printf("\nmatrix dimension does not divide by nprocs\n");
        MPI_Abort(MPI_COMM_WORLD,10);
    }
    //================= block sizes - end =================


    //================= memory allocation - start =================
    x       = (double *) (malloc(vsize * sizeof(double))); //global vector
    local_x = (double *) (malloc(vsize_local * sizeof(double)));
    local_A = (double *) malloc(rowsNum_local * colsNum * sizeof(double));
    y       = (double *) (malloc(rowsNum * sizeof(double))); //global vector
    local_y = (double *) (malloc(rowsNum_local * sizeof(double)));
    //================= memory allocation - end =================


    //================= local_A initialization - start =================
    //initialize local_A with columns as rows
    start_time = MPI_Wtime(); //start measuring time
    for (i = 0; i < rowsNum_local; i++ )
    {
        for (j = 0; j < colsNum; j++ )
        {
            local_A[i+j*rowsNum_local] = i + ((double)j + (double)my_rank * (double)rowsNum_local);
        }
    }
    //================= local_A initialization - end =================


    //================= local_x initialization - start =================
    for (j = 0; j < vsize_local; j++)
    {
        local_x[j] = (double)j + (double)my_rank * (double)vsize_local;
    }
    //================= local_x initialization - end =================


    //================= local computation - start =================
    MPI_Allgather(local_x, vsize_local, MPI_DOUBLE, x, vsize_local, MPI_DOUBLE, MPI_COMM_WORLD);//collect local vectors and combine them to a full vector

    dgemv_("N", &rowsNum_local, &colsNum, &d_one, local_A, &rowsNum_local, x, &one, &d_zero, local_y, &one); //local computation

    MPI_Gather(local_y, rowsNum_local, MPI_DOUBLE, y, rowsNum_local, MPI_DOUBLE, 0, MPI_COMM_WORLD); //send final result to Process 0 by order
    //================= local computation - end =================


    //================= print result - start =================
    if (my_rank == 0)
    {
        end_time = MPI_Wtime(); //stop measuring time
        printf("%dx%d matrix.\n%d processors.\nTime processing=%f.\n", rowsNum, colsNum,procsNum, end_time - start_time);
        printf("A x = \n");
        for (i = 0; i < rowsNum; i++)
        {
            printf("%d\t%f\n",i, y[i]);
        }
    }
    //================= print result - end =================


    //================= free memory - start =================
    free(local_A);
    free(x);
    free(local_x);
    free(y);
    free(local_y);
    //================= free memory - end =================

    MPI_Finalize();
}

