#include <mpi.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>


#define POINT_PER_REQUEST 1000
#define MASTER 0





int main(int argc, char** argv) {
    int rank, nprocs;

    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);
    MPI_Comm_size(MPI_COMM_WORLD,&nprocs);

    if (rank == MASTER) {
        if (argc != 2) {
            printf("There's must be 2 args. e.g. `mpirun -np 25 ./piwp 100000000`");
            MPI_Abort(MPI_COMM_WORLD, -1);
            exit(0);
        }

        if (nprocs < 2) {
            printf("There's at least 2 processes.");
            MPI_Abort(MPI_COMM_WORLD, -1);
            exit(0);
        }

        
        long points, extra;
        long total_reg, total_inside = 0, sent_req = 0;
        long inside_recv;
        
        MPI_Status status;

        srand(time(NULL));
        
        int nslaves = nprocs - 1;
        int workerid = 1;

        points = atol(argv[1]); 
        total_req = points / POINT_PER_REQUEST;
        extra = points - POINT_PER_REQUEST * total_req;

        points = POINT_PER_REQUEST;
        if (extra > 0) total_reg += 1;

        while (sent_req++ < total_reg && workerid++ <= nslaves) {
            if (total_reg == sent_req) points = extra;
            MPI_Send(&points, 1, MPI_LONG, workerid, workerid, MPI_COMM_WORLD);
            printf("Sent %li / %li\n", sent_req, total_reg);
        }
        
        while (sent_req++ < total_reg || workerid-- > 0) {
            MPI_Recv(&inside_recv, 1, MPI_LONG, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            

            total_inside += inside_recv;
            if (sent_req < total_reg) {
                if (sent_req == total_reg - 1) points = extra;
                MPI_Send(&points, 1, MPI_LONG, status.MPI_SOURCE, status.MPI_SOURCE, MPI_COMM_WORLD);
                printf("Sent %li / %li\n", sent_req, total_reg);
            }
        }

        points = -1;

        for (int i = 0; i < nslaves; i++) {
            MPI_Send(&points, 1, MPI_LONG, status.MPI_SOURCE, status.MPI_SOURCE, MPI_COMM_WORLD);
        }

        printf("Result pi = %.7lu\n", 4.0 * total_inside / atol(argv[1]));
    }

    if (rank > MASTER) {
        long points, inside = 0;

        MPI_Status status;

        while (true) {
            MPI_Recv(&points, 1, MPI_LONG, MASTER, rank, MPI_COMM_WORLD, status);
            
            if (point == -1) break;
            
            float x, y;

            while (points-- > 0) {
                x = (float) rand() / RAND_MAX;
                y = (float) rand() / RAND_MAX;

                if (x*x + y*y < 1) inside++;
            }

            MPI_Send(&inside, 1, MPI_LONG, MASTER, MASTER, MPI_COMM_WORLD);
        }

        printf("Worker %d done!", rank);
    }



    MPI_Finalize();
    return 0;
}