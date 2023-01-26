///////////////////////////////////////////
////////Nir Schneider: 316098052///////////
////////Wesam Salih: 211734504/////////////
///////////////////////////////////////////
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<mpi.h>


#define MASTER_TO_WORKER_TAG 1
#define MASTER_TO_WORKER_TAG_2 100
#define WORKER_TO_MASTER_TAG 200 
#define WORKER_TO_MASTER_TAG_2 300
#define WORKER_TO_MASTER_TAG_3 400


void printMatrix(int rows, int cols, int matrix[rows][cols]) //for validation purposes
{
    int i, j;
    for (i = 0; i < rows; i++)
    {
        for (j = 0; j < cols; j++) {
            printf("%d\t", matrix[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}
void fillMatrixB(int rows, int cols, int matrix[rows*cols], int dest[rows][cols])
{
    int i , j;
    for (i = 0; i < rows; i++)
    {
        for (j = 0; j < cols; j++) {
            dest[i][j] = matrix[i * cols + j];
        }
    }
}

void initMatrix(int rows, int cols, int matrix[rows][cols])
{
    int i = 0, j = 0;
    for (i = 0; i < rows; i++)
    {
        for (j = 0; j < cols; j++) {
            matrix[i][j] =0;
        }
    }
}


int main(int argc, char* argv[])
{
    //  A - original image matrix
    //  B - final processed image
    //  C_avg - processed image matrix of workers

    //=====================START - SCAN IMAGE===========================

    FILE *input_image,*output_image;

    input_image = fopen("input.pgm","r+");

    if(input_image == NULL)
    {
        printf("Error opening first file");
        exit(8);
    }

    char channel[2];
    fscanf(input_image, "%s\n", &channel);

    if (getc(input_image) == '#' )// If it is the case, ignore the second line in the input file
    {
        while(getc(input_image) != '\n');
    }
    else
    {
        fseek(input_image, -1, SEEK_CUR);
    }

    int rowsNum = 0;
    int colsNum = 0;
    int max_gray_scale = 0;
    fscanf(input_image,"%d", &rowsNum);// get columns number
    fscanf(input_image,"%d", &colsNum);// get rows number
    fscanf(input_image,"%d", &max_gray_scale);// get max gray scale of image

    int A[rowsNum][colsNum]; //original matrix representation of pgm file

    int i = 0, j = 0;//index to run over the matrix
    int curr_val;
    for (i=0; i < rowsNum; i++)
    {
        for (j=0; j < colsNum; j++)
        {
            fscanf(input_image,"%d", &curr_val);
            A[i][j] = curr_val;
        }
    }

    output_image = fopen("original","w+"); //for validation purpose
    for (i = 0 ; i < rowsNum ; i++ )
    {
        for (j = 0 ; j < colsNum ; j++ )
        {
            fprintf( output_image,"%d\t" , A[i][j] );
        }
        fprintf( output_image,"\n" );
    }

    fclose(output_image);
    fclose(input_image);

    //=====================END - SCAN IMAGE===========================



    //================= START - PROGRAM VARIABLES ===========================

    int rank; //process rank
    int size; //number of processes - above 2
    int avg = 0; //calculate neighbors average - will be the values for the new image
    int lowBound; //low bound of the number of rows of [A] allocated to a worker
    int upBound; //upper bound of the number of rows of [A] allocated to a worker
    int rows_portion; //portion of the number of rows of [A] allocated to a worker
    double startwtime = 0.0, endwtime = 0.0;
    //================= END - PROGRAM VARIABLES ===========================


    int B[rowsNum][colsNum];
    initMatrix(rowsNum, colsNum, B);

    int* C_avg = malloc((rowsNum * colsNum) * sizeof(int)); //dynamically allocate matrix in memory
    for (i = 0; i < rowsNum; i++)
    {
        for (j = 0; j < colsNum; j++)
        {
            C_avg[i * colsNum + j] = 0;
        }
    }


    //================= START - MPI SETTINGS ===========================

    MPI_Status status; //store status of MPI_Recv
    MPI_Init(&argc, &argv); //initialize MPI operations
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); //get the rank
    MPI_Comm_size(MPI_COMM_WORLD, &size); //get number of processes

    //================= END - MPI SETTINGS ===========================


    if (size == 1)
    {
        startwtime = MPI_Wtime();
        for (i = 0; i < rowsNum; i++) //iterate through a given set of rows of [A]
        {
            for (j = 0; j < colsNum; j++) //iterate through columns of [A]
            {
                if ((i == 0 && j == 0) || (i == 0 && j == colsNum - 1))// top corners
                    avg = j == 0 ? (A[i][j + 1] + A[i + 1][j + 1] + A[i][j] + A[i + 1][j]) / 4 : (A[i][j - 1] + A[i + 1][j - 1] + A[i][j] + A[i + 1][j]) / 4;
                else if ((i == rowsNum - 1 && j == 0) || (i == rowsNum - 1 && j == colsNum - 1)) //bottom corners
                    avg = j == 0 ? (A[i - 1][j + 1] + A[i][j + 1] + A[i - 1][j] + A[i][j]) / 4 : (A[i - 1][j] + A[i][j] + A[i - 1][j - 1] + A[i][j - 1]) / 4;
                else if (j == 0 || j == colsNum - 1) //left or right boundary
                    avg = j == 0 ? (A[i - 1][j + 1] + A[i][j + 1] + A[i + 1][j + 1] + A[i - 1][j] + A[i][j] + A[i + 1][j]) / 6 : (A[i - 1][j] + A[i][j] + A[i + 1][j] + A[i - 1][j - 1] + A[i][j - 1] + A[i + 1][j - 1]) / 6;
                else if (i == 0 || i == rowsNum - 1) //upper or lower boundary
                    avg = i == 0 ? (A[i][j + 1] + A[i + 1][j + 1] + A[i][j] + A[i + 1][j] + A[i][j - 1] + A[i + 1][j - 1]) / 6 : (A[i - 1][j + 1] + A[i][j + 1] + A[i - 1][j] + A[i][j] + A[i - 1][j - 1] + A[i][j - 1]) / 6;
                else //inner value
                    avg = (A[i - 1][j + 1] + A[i][j + 1] + A[i + 1][j + 1] + A[i - 1][j] + A[i][j] + A[i + 1][j] + A[i - 1][j - 1] + A[i][j - 1] + A[i + 1][j - 1]) / 9;

                C_avg[i * colsNum + j] = avg; //C_avg-new image
            }
        }
        endwtime = MPI_Wtime();
        printf("\nWall clock time = %f\n", endwtime - startwtime);
        fillMatrixB(rowsNum, colsNum, C_avg, B);

        //===================== START - EXPORT PROCESSED IMAGE ===============================

        FILE* output_image;
        output_image = fopen("output.pgm", "wb");
        fprintf(output_image, "P2\n");
        fprintf(output_image, "%d %d\n", rowsNum, colsNum);
        fprintf(output_image, "255\n");
        for (i = 0; i < rowsNum; i++) {
            for (j = 0; j < colsNum; j++) {
                fprintf(output_image, "%d\n", B[i][j]);
            }
            fprintf(output_image, "");
        }

        //===================== END - EXPORT PROCESSED IMAGE ===============================


    }
    else
    {
        //=========================== START - MASTER SEND ==========================================
        if (rank == 0) //master's work definition
        {
            rows_portion = (rowsNum / (size - 1)); //calculate portion without master
            startwtime = MPI_Wtime();
            for (i = 1; i < size; i++) //for each worker
            {
                lowBound = (i - 1) * rows_portion; //send the low bound for each worker
                if ((i + 1 == size) && (rowsNum % (size - 1) != 0)) //if rows of [A] cannot be equally divided among workers
                    upBound = rowsNum; //last worker gets all the remaining rows
                else
                    upBound = lowBound + rows_portion; //rows of [A] are equally divided among workers

                MPI_Send(&lowBound, 1, MPI_INT, i, MASTER_TO_WORKER_TAG + i, MPI_COMM_WORLD); //send the low bound to the i worker
                MPI_Send(&upBound, 1, MPI_INT, i, MASTER_TO_WORKER_TAG_2 + i, MPI_COMM_WORLD); //send the upper bound to the i worker
            }

            //=========================== END - MASTER SEND ==========================================

            //=========================== START - MASTER RECEIVE ==========================================

            for (i = 1; i < size; i++)// until all workers have handed back the processed data
            {
                MPI_Recv(&lowBound, 1, MPI_INT, i, WORKER_TO_MASTER_TAG_2 + i, MPI_COMM_WORLD, &status);//receive processed data from a worker
                MPI_Recv(&upBound, 1, MPI_INT, i, WORKER_TO_MASTER_TAG_3 + i, MPI_COMM_WORLD, &status);//receive processed data from a worker
                MPI_Recv(&C_avg[lowBound * colsNum], (upBound - lowBound) * colsNum, MPI_INT, i, WORKER_TO_MASTER_TAG+i, MPI_COMM_WORLD, &status);//receive processed data from a worker
            }
            fillMatrixB(rowsNum, colsNum, C_avg, B);
            endwtime = MPI_Wtime();
            printf("\nWall clock time = %f\n", endwtime - startwtime);

            //=========================== END - MASTER RECEIVE ==========================================

            //===================== START - EXPORT PROCESSED IMAGE ===============================

            FILE* output_image;
            output_image = fopen("output.pgm", "wb");
            fprintf(output_image, "P2\n");
            fprintf(output_image, "%d %d\n", rowsNum, colsNum);
            fprintf(output_image, "255\n");
            for (i = 0; i < rowsNum; i++) {
                for (j = 0; j < colsNum; j++) {
                    fprintf(output_image, "%d\n", B[i][j]);
                }
                fprintf(output_image, "");
            }

            //===================== END - EXPORT PROCESSED IMAGE ===============================

        }
        else
        {
            MPI_Recv(&lowBound, 1, MPI_INT, 0, MASTER_TO_WORKER_TAG + rank, MPI_COMM_WORLD, &status);//receive low bound from the master
            MPI_Recv(&upBound, 1, MPI_INT, 0, MASTER_TO_WORKER_TAG_2 + rank, MPI_COMM_WORLD, &status);//receive upper bound from the master
            for (i = lowBound; i < upBound; i++) //iterate through a given set of rows of [A]
            {
                for (j = 0; j < colsNum; j++) //iterate through columns of [A]
                {
                    if ((i == 0 && j == 0) || (i == 0 && j == colsNum - 1))// top corners
                        avg = j == 0 ? (A[i][j + 1] + A[i + 1][j + 1] + A[i][j] + A[i + 1][j]) / 4 : (A[i][j - 1] + A[i + 1][j - 1] + A[i][j] + A[i + 1][j]) / 4;
                    else if ((i == rowsNum - 1 && j == 0) || (i == rowsNum - 1 && j == colsNum - 1)) //bottom corners
                        avg = j == 0 ? (A[i - 1][j + 1] + A[i][j + 1] + A[i - 1][j] + A[i][j]) / 4 : (A[i - 1][j] + A[i][j] + A[i - 1][j - 1] + A[i][j - 1]) / 4;
                    else if (j == 0 || j == colsNum - 1) //left or right boundary
                        avg = j == 0 ? (A[i - 1][j + 1] + A[i][j + 1] + A[i + 1][j + 1] + A[i - 1][j] + A[i][j] + A[i + 1][j]) / 6 : (A[i - 1][j] + A[i][j] + A[i + 1][j] + A[i - 1][j - 1] + A[i][j - 1] + A[i + 1][j - 1]) / 6;
                    else if (i == 0 || i == rowsNum - 1) //upper or lower boundary
                        avg = i == 0 ? (A[i][j + 1] + A[i + 1][j + 1] + A[i][j] + A[i + 1][j] + A[i][j - 1] + A[i + 1][j - 1]) / 6 : (A[i - 1][j + 1] + A[i][j + 1] + A[i - 1][j] + A[i][j] + A[i - 1][j - 1] + A[i][j - 1]) / 6;
                    else //inner value
                        avg = (A[i - 1][j + 1] + A[i][j + 1] + A[i + 1][j + 1] + A[i - 1][j] + A[i][j] + A[i + 1][j] + A[i - 1][j - 1] + A[i][j - 1] + A[i + 1][j - 1]) / 9;

                    C_avg[i * colsNum + j] = avg; //C_avg-new image
                }
            }
            MPI_Send(&lowBound , 1, MPI_INT, 0, WORKER_TO_MASTER_TAG_2 + rank, MPI_COMM_WORLD);//send the processed portion of data to the master
            MPI_Send(&upBound,1, MPI_INT, 0, WORKER_TO_MASTER_TAG_3 + rank, MPI_COMM_WORLD);//send the processed portion of data to the master
            MPI_Send(&C_avg[lowBound * colsNum ], (upBound - lowBound) * colsNum, MPI_INT, 0, WORKER_TO_MASTER_TAG+rank, MPI_COMM_WORLD);//send the processed portion of data to the master

        }
    }
    MPI_Finalize(); //finalize MPI operations
    free(C_avg);
    return 0;
}