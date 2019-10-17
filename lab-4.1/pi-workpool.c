#include <mpi.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define POINT_PER_REQUEST 1000
#define MASTER 0

int main(int argc, char** argv) {
    int rank, nprocs;

    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);
    MPI_Comm_size(MPI_COMM_WORLD,&nprocs);

    if (rank == MASTER) {
        if (argc < 2) {
            printf("There's must be 2 args. e.g. `mpirun -np 25 ./piwp 100000000`");
            MPI_Abort(MPI_COMM_WORLD, -1);
            exit(0);
        }

        if (nprocs < 2) {
            printf("There's at least 2 processes.");
            MPI_Abort(MPI_COMM_WORLD, -1);
            exit(0);
        }
        
        int DEBUG = 0;
        
        long points, extra;
        long total_req, total_inside = 0, sent_req = 0;
        long inside_recv;
        
        MPI_Status status;
        
        double start = MPI_Wtime();
        
        int nslaves = nprocs - 1;
        int workerid = 1;

        points = atol(argv[1]); 
        total_req = points / POINT_PER_REQUEST;
        extra = points - POINT_PER_REQUEST * total_req;

        points = POINT_PER_REQUEST;
        if (extra > 0) total_req += 1;
        
        

        while (sent_req < total_req && workerid <= nslaves) {
            if (total_req == sent_req) points = extra;
            MPI_Send(&points, 1, MPI_LONG, workerid, workerid, MPI_COMM_WORLD);
            sent_req++; workerid++;
            if (DEBUG) printf("Sent %li / %li\n", sent_req, total_req);
        }

        printf("All %d workers received!\n", workerid - 1);
        
        while (sent_req < total_req || workerid-- > 1) {
            MPI_Recv(&inside_recv, 1, MPI_LONG, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            total_inside += inside_recv;
            if (sent_req < total_req) {
                if (sent_req == total_req - 1 && extra > 0) points = extra;
                MPI_Send(&points, 1, MPI_LONG, status.MPI_SOURCE, status.MPI_SOURCE, MPI_COMM_WORLD);
                sent_req++;
                if (DEBUG) printf("Sent %li / %li\n", sent_req, total_req);
            }
        
        }
        double finish = MPI_Wtime();
        printf("Job done id %.7lfs! Send message anoucing terminate!\n", finish - start);
        points = -1;

        for (int i = 1; i <= nslaves; i++) {
            MPI_Send(&points, 1, MPI_LONG, i, i, MPI_COMM_WORLD);
        }
        printf("Inside %li of %li\n", total_inside, atol(argv[1]));
        printf("Result pi = %.7lf\n", 4.0 * total_inside/atol(argv[1]));
    }

    if (rank > MASTER) {
        long points, inside = 0;
        
        MPI_Status status;

        srand(time(NULL));
        
        while (1) {
            inside = 0;
            
            MPI_Recv(&points, 1, MPI_LONG, MASTER, rank, MPI_COMM_WORLD, &status);
            
            if (points == -1) break;
            
            float x, y;

            while (points-- > 0) {
                x = (float) rand() / RAND_MAX;
                y = (float) rand() / RAND_MAX;

                if (x*x + y*y < 1) inside++;
            }

            MPI_Send(&inside, 1, MPI_LONG, MASTER, MASTER, MPI_COMM_WORLD);

        }

    }



    MPI_Finalize();
    return 0;
}
