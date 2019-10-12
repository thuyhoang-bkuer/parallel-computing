#include "mpi.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MASTER 0


int masterWork(int argc, char** argv, int nprocs, int DEBUG = 0) {
    int nslaves;
    long points, extra;
    long total, inside = 0;

    if (argc != 2) {
        printf("There's must be 2 args. e.g. `mpirun -np 25 ./pims 100000000`");
        MPI_Abort(MPI_COMM_WORLD, -1);
        exit(0);
    }

    if (nprocs < 2) {
        printf("There's at least 2 processes.");
        MPI_Abort(MPI_COMM_WORLD, -1);
        exit(0);
    }

    nslaves = nprocs - 1;
    total = atoi(argv[1]);
    
    points = total / nslaves;
    extra = total - points * nslaves;
    
    MPI_Bcast(&points, 1, MPI_LONG, MASTER, MPI_COMM_WORLD);
    MPI_Bcast(&extra, 1, MPI_LONG, MASTER, MPI_COMM_WORLD);

    if (DEBUG) printf("Broadcast successfully!\n");

    printf("Waiting for slaves..\n");

    MPI_Reduce(&inside, &inside, 1, MPI_LONG, MPI_SUM, MASTER, MPI_COMM_WORLD);

    double pi = ((double) inside) / total;
    printf("pi = %.7lf\n", pi);
}


int slaveWork(int rank, int DEBUG = 0) {
    long points, extra;
    long inside;

    if (rank <= extra) points += 1;

    MPI_Bcast(&points, 1, MPI_LONG, MASTER, MPI_COMM_WORLD);
    MPI_Bcast(&extra, 1, MPI_LONG, MASTER, MPI_COMM_WORLD);

    if (DEBUG) printf("Slave %d received successfully!\n", rank);

    float x, y;

    while (points-- > 0) {
        x = (float) rand() / RAND_MAX;
        y = (float) rand() / RAND_MAX;

        if (x*x + y*y < 1) inside++;
    }

    MPI_Reduce(&inside, &inside, 1, MPI_LONG, MPI_SUM, MASTER, MPI_COMM_WORLD);
}




int main(int argc, char** argv) {
    int rank, nprocs;


    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);
    MPI_Comm_size(MPI_COMM_WORLD,&nprocs);
    
    srand(time(NULL));

    if (rank == 0) 
        masterWork(argc, argv, nprocs);
    else
        slaveWork(rank);

    MPI_Finalize();
    return 0;
}