#include <mpi.h>
#include <math.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char ** argv) {
    int rank, nproc, root = 0;
    long irecv = 0;
    unsigned long npoint, inside = 0;
    float x, y;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nproc);    
    if (rank == 0) {
        if (argc != 2) {
            printf("There must be 2 args!\n");
            printf("E.g: mpirun -np 50 ./pi 1000000000\n");
            MPI_Finalize();
            return -1;
        }

        srand(time(NULL));
    }

    npoint = atol(argv[1]) / nproc;
    

    while (npoint-- > 0) {

        x =(float) rand() / RAND_MAX;
        y =(float) rand() / RAND_MAX;
        
        // Uncomment line below for debug
        // printf("(%.2f, %.2f) ", x, y);

        if (x*x + y*y <= 1) inside++;
    }

    MPI_Reduce(&inside, &irecv, 1, MPI_LONG, MPI_SUM, root, MPI_COMM_WORLD);


    if (rank == 0) {
        printf("Total: %lu / %s\n", irecv, argv[1]);
        printf("Pi = %.9lf\n", 4.0 * irecv / atol(argv[1]));
    }

    MPI_Finalize();

	return 0;
}
