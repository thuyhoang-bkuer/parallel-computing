#include <mpi.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>



#define MASTER 0              // Rank of Master's proc
#define FROM_MASTER 1         // Message's tag from master to worker
#define FROM_WORKER 2         // Messaeg's tag from worker to master

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

  double **A, **B, **R;                // Matrice
  MPI_Status status;
  char *filenameA = "100x100.matA",
       *filenameB = "100x100.matB",
       *filenameR = "p2p.matR";
  int rowA, rowB, colA, colB;
  int DEBUG = 0;
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &taskid);
  MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    
  numworkers = numtasks - 1;
  

  /*
    * Master task
  */
  if (taskid == MASTER) {
      if (argc == 2) {
        DEBUG = atoi(argv[1]);  
      }

      if (numtasks < 2) {
               printf("Need at least two MPI tasks\n");
                    MPI_Abort(MPI_COMM_WORLD, rc);
                         return -1;
                            
      }
    
    
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
        printf("matA read from %s. matB read from %s.\n",filenameA, filenameB);
        printf("matA: %dx%d. matB: %dx%d.", rowA, colA, rowB, colB);
    }
    
    if (colA != rowB) {
        printf("AxB is unmultiplicable.\n");
        MPI_Abort(MPI_COMM_WORLD, rc);
        return -1;
    }
    printf("resultMat: %dx%d.\n", rowA, colB);
    // Initialize matrix
    A = (double **) malloc(rowA * sizeof(double *));
    B = (double **) malloc(rowB * sizeof(double *));
    R = (double **) malloc(rowA * sizeof(double *));
    
    
    for (i = 0; i < rowA; i++) {
      A[i] = (double *) malloc(colA * sizeof(double));
      R[i] = (double *) malloc(colB * sizeof(double));
      for (j = 0; j < colA; j++) {
        fscanf(matA, "%lf", &A[i][j]);
      }
    }

    for (i = 0; i < rowB; i++) {
        B[i] = (double *) malloc(colB * sizeof(double));
        for (j = 0; j < colB; j++) {
            fscanf(matB, "%lf", &B[i][j]);
        }
    }

    fclose(matA);
    fclose(matB);

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
    
    // Measure start time
    double start = MPI_Wtime();

    // Send matrix data to the workers
    averagerows = rowA / numworkers;
    extra = rowA % numworkers;
    offset = 0;
    mtype = FROM_MASTER;

    for (dest = 1; dest <= numworkers; dest++) {
        rows = (dest <= extra) ? averagerows + 1 : averagerows;
        // Sending info to workers
        MPI_Send(&offset, 1, MPI_INT, dest, mtype, MPI_COMM_WORLD);
        MPI_Send(&rows, 1, MPI_INT, dest, mtype, MPI_COMM_WORLD);
        MPI_Send(&colA, 1, MPI_INT, dest, mtype, MPI_COMM_WORLD);
        MPI_Send(&colB, 1, MPI_INT, dest, mtype, MPI_COMM_WORLD);
        for (i = offset; i < offset + rows; i++) {
            MPI_Send(&A[i][0], colA, MPI_DOUBLE, dest, mtype, MPI_COMM_WORLD);
        }
        for (i = 0; i < colA; i++) {
            MPI_Send(&B[i][0], colB, MPI_DOUBLE, dest, mtype, MPI_COMM_WORLD);
        }
        
        offset += rows;

        // Debug
        if (DEBUG) printf("Successfully sending to worker %d.\n", dest);
    }

    // Waiting the results from those workers.
    printf("Waiting for workers..\n");
    mtype = FROM_WORKER;
    offset = 0;
    for (source = 1; source <= numworkers; source++) {
        MPI_Recv(&offset, 1, MPI_INT, source, mtype, MPI_COMM_WORLD, &status);
        MPI_Recv(&rows, 1, MPI_INT, source, mtype, MPI_COMM_WORLD, &status);
        for (i = offset; i < rows + offset; i++) {
            MPI_Recv(&R[i][0], colB, MPI_DOUBLE, source, mtype, MPI_COMM_WORLD, &status);
        }
        
        // Debug
        if (DEBUG) printf("Successfully receiving from workers %d.\n", source);
    
    }
    
    double finish = MPI_Wtime();
    printf("Done in %.7f seconds.\n", finish - start);
    
    // Writing result
    printf("Writing result to.. %s\n", filenameR);
    FILE* result = fopen(filenameR, "w+");
    fprintf(result, "%d %d\n", rowA, colB);
    for (i = 0; i < rowA; i++) {
        for (j = 0; j < colB; j++) {
            fprintf(result, "%.2lf ", R[i][j]);
        }
        fprintf(result, "\n");
    }
    fclose(result);

    // Deallocating
    for (i = 0; i < rowA; i++) {
      free(A[i]);
      free(R[i]);
    }
    for (i = 0; i < rowB; i++) {
      free(B[i]);
    }
    free(A); free(B); free(R);
  }  


  if (taskid > MASTER) {
    // Receiving data from master
    mtype = FROM_MASTER;
    MPI_Recv(&offset, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD, &status);
    MPI_Recv(&rows, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD, &status);
    MPI_Recv(&colA, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD, &status);
    MPI_Recv(&colB, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD, &status);
    A = (double **) malloc(rows * sizeof(double *));
    R = (double **) malloc(rows * sizeof(double *));
    B = (double **) malloc(colA * sizeof(double *));

    for (i = 0; i < rows; i++) {
      A[i] = (double *) malloc(colA * sizeof(double));
      R[i] = (double *) malloc(colB * sizeof(double));
      MPI_Recv(&A[i][0], colA, MPI_DOUBLE, MASTER, mtype, MPI_COMM_WORLD, &status);
    }

    for (i = 0; i < colA; i++) {
      B[i] = (double *) malloc(colB * sizeof(double));
      MPI_Recv(&B[i][0], colB, MPI_DOUBLE, MASTER, mtype, MPI_COMM_WORLD, &status);
    }

    if(DEBUG) printf("Worker %d successfully received data!\n", taskid);

    // Do the job!!
    for (i = 0; i < rows; i++) {
      for (j = 0; j < colB; j++) {
        R[i][j] = 0.0;
        for (k = 0; k < colA; k++) {
          R[i][j] += A[i][k] * B[k][j];
        }
      }
    }

    // Sending data to master
    mtype = FROM_WORKER;
    MPI_Send(&offset, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD);
    MPI_Send(&rows, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD);
    for (i = 0; i < rows; i++) {
      MPI_Send(&R[i][0], colB, MPI_DOUBLE, MASTER, mtype, MPI_COMM_WORLD);
    }
  }

  MPI_Finalize();

  return 0;
}

