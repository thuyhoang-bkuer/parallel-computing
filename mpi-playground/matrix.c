#include <mpi.h>
#include <stdio.h>
#include <math.h>
#include <string.h>


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

  double **A, **B;                // Matrice

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &taskid);
  MPI_Comm_size(MPI_COMM_WORLD, &numtasks);

  if (argc != 2) {
    printf("There must be 2 arguments!\n");
    MPI_Abort(MPI_COMM_WORLD, rc);
    exit(1);
  }

  if (numtasks < 2) {
    printf("Need at least two MPI tasks\n");
    MPI_Abort(MPI_COMM_WORLD, rc);
    exit(1);
  }

  /*
    * Master task
  */
  if (taskid == MASTER) {
    const INT D = stoi(argv[1]);
    printf("MPI has started with %d tasks.\n", numtasks);

    // Initialize matrix
    A = B = (double **) malloc(SIZE * sizeof(double *));
    for (i = 0; i < SIZE; i++) {
      A[i] = B[i] = (double *) malloc(SIZE * sizeof(double));
      for (j = 0; j < SIZE; j++) {
        A[i][j] = i + j;
        B[i][j] = i * j;
      }
    }

    // Print for visualize
    if (SIZE > 3) {
      printSample(A, 3, 3);
      printSample(B, 3, 3);
    }
    else {
      printSample(A, SIZE, SIZE);
      printSample(B, SIZE, SIZE);
    }


  } 
}

