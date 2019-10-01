#include <mpi.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>



#define ROOT 0              // Rank of ROOT's proc
#define FROM_MASTER 1         // Message's tag from ROOT to worker
#define FROM_WORKER 2         // Messaeg's tag from worker to ROOT

void printSample(double ** mat, int row, int col) {
  for (int i = 0; i < row; i++) {
    printf("[ ");
    for (int j = 0; j < col; j++) {
      printf("%lf ", mat[i][j]);
    }
    printf("]\n");
  }
}

int main(int argc, char** argv) {

  // Defining variables
  int numtasks,                   // Number of task in the context
      taskid,                     // Task identifier
      numworkers,                 // Number of workers
      source,                     // Task' id of message's source
      dest,                       // Task's id of message's destination
      mtype,                      // Message type
      rows,                       // Number of rows
      averagerows, extra, offset, // Arguments for determining rows send to each worker
      i, j, k, rc;                // misc

  double *sendA, *A, *B, *recvR, *R;                // Matrice
  double start, finish;           // Measure time
  char *filenameA = "matA.txt",
       *filenameB = "matB.txt",
       *filenameR = "result.txt";
  int rowA, rowB, colA, colB;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &taskid);
  MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    
  numworkers = numtasks - 1;
  

  /*
    * ROOT task
  */
  if (taskid == ROOT) {
      if (argc > 1) {
                printf("There must be 1 arguments!\n");
                            
      }

      if (numtasks < 2) {
               printf("Need at least two MPI tasks\n");
                    MPI_Abort(MPI_COMM_WORLD, rc);
                         return -1;
                            
      }
    
      const int DEBUG = 1;
    
    printf("MPI has started with %d tasks.\n", numtasks);
    
    // Read data
    FILE *matA = fopen(filenameA, "r");
    FILE *matB = fopen(filenameB, "r");
    
    if (!matA || !matB) {
        printf("File not found.\n");
        MPI_Abort(MPI_COMM_WORLD, rc);
        return -1;
    }
    else {
        fscanf(matA, "%d %d", &rowA, &colA);
        fscanf(matB, "%d %d", &rowB, &colB);
        printf("matA: %dx%d. matB: %dx%d.", rowA, colA, rowB, colB);
    }

    if (colA != rowB) {
        printf("AxB is unmultiplicable.\n");
        MPI_Abort(MPI_COMM_WORLD, rc);
        return -1;
    }
    printf("resultMat: %dx%d.\n", rowA, colB);
    // Initialize matrix
    sendA = (double *) malloc(rowA * colA * sizeof(double));
    B = (double *) malloc(rowB * colB * sizeof(double));
    recvR = (double *) malloc(rowA * colB * sizeof(double));
    
    
    for (i = 0; i < rowA; i++) {
      for (j = 0; j < colA; j++) {
        fscanf(matA, "%lf", &A[rowA * i + j]);
      }
    }

    for (i = 0; i < rowB; i++) {
        for (j = 0; j < colB; j++) {
            fscanf(matB, "%lf", &B[rowB * i + j]);
        }
    }

    fclose(matA);
    fclose(matB);

    rows = (rowA + numtasks - 1)/ numtasks;
    printf("Each task computes %d row(s).\n", rows);

    // Print for visualize
    if (DEBUG)
        if (rowA > 10 || rowB > 10) {
            printSample(A, 10, 10);
            printSample(B, 10, 10);
        }
        else {
            printSample(A, rowA, colA);
            printSample(B, rowB, colB);
        }

    start = MPI_Wtime();
  }
  
  // Starting point
  MPI_Barrier(MPI_COMM_WORLD);

  // Broadcast metrics
  MPI_Bcast(&rowA, 1, MPI_INT, ROOT,MPI_COMM_WORLD);
  MPI_Bcast(&colA, 1, MPI_INT, ROOT,MPI_COMM_WORLD);
  MPI_Bcast(&rowB, 1, MPI_INT, ROOT,MPI_COMM_WORLD);
  MPI_Bcast(&colB, 1, MPI_INT, ROOT,MPI_COMM_WORLD);
  MPI_Bcast(&rows, 1, MPI_INT, ROOT,MPI_COMM_WORLD);

  // Broadcast matrix B to all processors
  if (!B) B = (double *) malloc(rowb * colB);
  MPI_Bcast(&B, rowB * colB, MPI_DOUBLE, ROOT, MPI_COMM_WORLD);

  // Send matrix data to the workers
  A = (double *) malloc(rows * colA * sizeof(double));
  MPI_Scatter(sendA, rows * colA, MPI_DOUBLE, A, rows * colA, MPI_DOUBLE, ROOT, MPI_COMM_WORLD);

  // Do the job
  R = (double *) malloc(rows * colB * sizeof(double));
  for (i = 0; i < rows; i++) {
    for (j = 0; j < colB; j++) {
      position = i * rows + j;
      R[position] = 0.0;
      for (k = 0; k < colA; k++) {
        R[position] += A[i * rows + k] * B[k * colB + j];
      }
    }
  }

  MPI_Gather(R, rows * colB, MPI_DOUBLE, recvR, rows * colB, MPI_DOUBLE, ROOT, MPI_COMM_WORLD);

  // Synchronize data
  MPI_Barrier(MPI_COMM_WORLD);

  if (taskid == ROOT) {
    finish = MPI_Wtime();
    printf("Job done in %.4f.\n", finish - start);

    // Writing result
    FILE* result = fopen(filenameR, "w+");
    fprintf(result, "%d %d\n", rowA, colB);
    for (i = 0; i < rowA; i++) {
        for (j = 0; j < colB; j++) {
            fprintf(result, "%lf ", R[i* rows + j]);
        }
        fprintf(result, "\n");
    }
    fclose(result);
  }

  MPI_Finalize();

  return 0;
}

