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
  const int D = atoi(argv[1]);

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &taskid);
  MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    
  numworkers = numtasks - 1;
  

  /*
    * Master task
  */
  if (taskid == MASTER) {
      if (argc != 2) {
               printf("There must be 2 arguments!\n");
                    MPI_Abort(MPI_COMM_WORLD, rc);
                         return -1;
                            
      }

      if (numtasks < 2) {
               printf("Need at least two MPI tasks\n");
                    MPI_Abort(MPI_COMM_WORLD, rc);
                         return -1;
                            
      }

    printf("MPI has started with %d tasks.\n", numtasks);

    // Initialize matrix
    A = (double **) malloc(D * sizeof(double *));
    B = (double **) malloc(D * sizeof(double *));
    R = (double **) malloc(D * sizeof(double *));
    for (i = 0; i < D; i++) {
      A[i] = (double *) malloc(D * sizeof(double));
      B[i] = (double *) malloc(D * sizeof(double));
      R[i] = (double *) malloc(D * sizeof(double));
      for (j = 0; j < D; j++) {
        A[i][j] = (double) (i + j) / (2 * D);
        B[i][j] = (double) (i * j) / (D * D);
      }
    }

    // Print for visualize
    if (D > 3) {
      printSample(A, 3, 3);
      printSample(B, 3, 3);
    }
    else {
      printSample(A, D, D);
      printSample(B, D, D);
    }
    
    // Measure start time
    double start = MPI_Wtime();

    // Send matrix data to the workers
    averagerows = D / numworkers;
    extra = D % numworkers;
    offset = 0;
    mtype = FROM_MASTER;

    for (dest = 1; dest <= numworkers; dest++) {
        rows = (dest <= extra) ? averagerows + 1 : averagerows;
        // Sending info to workers
        MPI_Send(&offset, 1, MPI_INT, dest, mtype, MPI_COMM_WORLD);
        MPI_Send(&rows, 1, MPI_INT, dest, mtype, MPI_COMM_WORLD);
        for (i = offset; i < offset + rows; i++) {
            MPI_Send(&A[i][0], D, MPI_DOUBLE, dest, mtype, MPI_COMM_WORLD);
        }
        for (i = 0; i < D; i++) {
            MPI_Send(&B[i][0], D, MPI_DOUBLE, dest, mtype, MPI_COMM_WORLD);
        }
        
        offset += rows;

        // Debug
        printf("Successfully sending to worker %d.\n", dest);
    }

    // Waiting the results from those workers.
    printf("Waiting for workers..\n");
    mtype = FROM_WORKER;
    offset = 0;
    for (source = 1; source <= numworkers; source++) {
        MPI_Recv(&offset, 1, MPI_INT, source, mtype, MPI_COMM_WORLD, &status);
        MPI_Recv(&rows, 1, MPI_INT, source, mtype, MPI_COMM_WORLD, &status);
        for (i = offset; i < rows + offset; i++) {
            MPI_Recv(&R[i][0], D, MPI_DOUBLE, source, mtype, MPI_COMM_WORLD, &status);
        }
        
        // Debug
        printf("Successfully receiving from workers %d.\n", source);
    
    }
    
    double finish = MPI_Wtime();
    printf("Done in %.2f seconds.\n", finish - start);

    // Debug - print result
    printSample(R, 11, 11);
  }

  if (taskid > MASTER) {
    // Receiving data from master
    mtype = FROM_MASTER;
    MPI_Recv(&offset, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD, &status);
    MPI_Recv(&rows, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD, &status);

    A = (double **) malloc(rows * sizeof(double *));
    R = (double **) malloc(rows * sizeof(double *));
    B = (double **) malloc(D * sizeof(double *));

    for (i = 0; i < rows; i++) {
      A[i] = (double *) malloc(D * sizeof(double));
      R[i] = (double *) malloc(D * sizeof(double));
      MPI_Recv(&A[i][0], D, MPI_DOUBLE, MASTER, mtype, MPI_COMM_WORLD, &status);
    }

    for (i = 0; i < D; i++) {
      B[i] = (double *) malloc(D * sizeof(double));
      MPI_Recv(&B[i][0], D, MPI_DOUBLE, MASTER, mtype, MPI_COMM_WORLD, &status);
    }

    printf("Worker %d successfully received data!\n", taskid);

    // Do the job!!
    for (i = 0; i < rows; i++) {
      for (j = 0; j < D; j++) {
        R[i][j] = 0.0;
        for (k = 0; k < D; k++) {
          R[i][j] += A[i][k] * B[k][j];
        }
      }
    }

    // Sending data to master
    mtype = FROM_WORKER;
    MPI_Send(&offset, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD);
    MPI_Send(&rows, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD);
    for (i = 0; i < rows; i++) {
      MPI_Send(&R[i][0], D, MPI_DOUBLE, MASTER, mtype, MPI_COMM_WORLD);
    }
  }

  MPI_Finalize();

  return 0;
}

