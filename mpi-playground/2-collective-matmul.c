#include <mpi.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>



#define ROOT 0              // Rank of ROOT's proc
#define FROM_MASTER 1         // Message's tag from ROOT to worker
#define FROM_WORKER 2         // Messaeg's tag from worker to ROOT

void printSample(double * mat, int row, int col) {
  for (int i = 0; i < row; i++) {
    printf("[ ");
    for (int j = 0; j < col; j++) {
      printf("%lf ", mat[i*col + j]);
    }
    printf("]\n");
  }
}

int main(int argc, char** argv) {

  // Defining variables
  int numtasks,                   // Number of task in the context
      taskid,                     // Task identifier
      rows,                       // Number of rows
      i, j, k, rc;                // misc

  double *sendA, *A, *B, *recvR, *R;                // Matrice
  double start, finish;           // Measure time
  char *filenameA = "matA.txt",
       *filenameB = "matB.txt",
       *filenameR = "result.txt";
  int rowA, rowB, colA, colB;
  int DEBUG = 0;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &taskid);
  MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    
  /*
    * ROOT task
  */
  if (taskid == ROOT) {
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
        printf("matA: %dx%d. matB: %dx%d.", rowA, colA, rowB, colB);
    }

    if (colA != rowB) {
        printf("AxB is unmultiplicable.\n");
        MPI_Abort(MPI_COMM_WORLD, rc);
        return -1;
    }
    printf(" => resultMat: %dx%d.\n", rowA, colB);
    
    rows = (rowA + numtasks - 1)/ numtasks;
    printf("Each task computes %d row(s).\n", rows);
    
    // Initialize matrix
    sendA = (double *) calloc(rows * numtasks * colA + 1,sizeof(double));
    B = (double *) malloc(rowB * colB * sizeof(double));
    recvR = (double *) calloc(rows * numtasks * colB,sizeof(double));
    
    
    for (i = 0; i < rowA; i++) {
      for (j = 0; j < colA; j++) {
        fscanf(matA, "%lf", &sendA[colA * i + j]);
      }
    }

    for (i = 0; i < rowB; i++) {
        for (j = 0; j < colB; j++) {
            fscanf(matB, "%lf", &B[colB * i + j]);
        }
    }

    fclose(matA);
    fclose(matB);

    // Print for visualize
    if (DEBUG)
        if (rowA > 10 || rowB > 10) {
            printSample(sendA, 10, 10);
            printSample(B, 10, 10);
        }
        else {
            printSample(sendA, rows * numtasks, colA);
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
  if (DEBUG == 1 && taskid == ROOT) printf("Broadcast metrics successfully!\n");
  
  // Broadcast matrix B to all processors
  if (B == NULL) B = (double *) malloc(rowB * colB * sizeof(double));
  MPI_Bcast(B, rowB * colB, MPI_DOUBLE, ROOT, MPI_COMM_WORLD);
  if (DEBUG == 1 && taskid == ROOT) printf("Broadcast matrix B successfully!\n");

  // Send matrix data to the workers
  A = (double *) malloc(rows * colA * sizeof(double));
  MPI_Scatter(sendA, rows * colA, MPI_DOUBLE, A, rows * colA, MPI_DOUBLE, ROOT, MPI_COMM_WORLD);
  MPI_Comm_rank(MPI_COMM_WORLD, &taskid); 
  if (DEBUG && taskid == 1) {
    printf("After scatter  [%d]: \n", taskid);
    printSample(A, 1, colA);
    printSample(B, rowB, colB);
  }
  
  // Do the job
  R = (double *) malloc(rows * colB * sizeof(double));
  int position;
  for (i = 0; i < rows; i++) {
    for (j = 0; j < colB; j++) {
      position = i * rows + j;
      R[position] = 0.0;
      for (k = 0; k < colA; k++) {
        
        R[position] += A[i * rows + k] * B[k * colB + j];
      }
    }
  }

  //if (DEBUG && taskid == 0) printSample(R, rows, colB);

  MPI_Gather(R, rows * colB, MPI_DOUBLE, recvR, rows * colB, MPI_DOUBLE, ROOT, MPI_COMM_WORLD);

  // Synchronize data
  MPI_Barrier(MPI_COMM_WORLD);
  
  if (DEBUG && taskid == 0) { 
      printf("recvR[%d]: \n", taskid);
      printSample(recvR, rowA, colB);
  }
  
  if (taskid == ROOT) {
    finish = MPI_Wtime();
    printf("Job done in %.4f.\n", finish - start);

    // Writing result
    FILE* result = fopen(filenameR, "w+");
    fprintf(result, "%d %d\n", rowA, colB);
    for (i = 0; i < rowA; i++) {
        for (j = 0; j < colB; j++) {
            fprintf(result, "%.3lf ", recvR[i*colB + j]);
        }
        fprintf(result, "\n");
    }
    fclose(result);
  }

  MPI_Finalize();

  return 0;
}

