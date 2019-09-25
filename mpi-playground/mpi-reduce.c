#include <mpi.h>
#include <math.h>
#include <stdio.h>


int main(int argc, char** argv) {
	int rank, nproc;
	int irecv[2];
	
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &nproc);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	int isend[] = {rank, -rank};
	// printf("rank %d isend[] = %d\n", rank, isend);
	MPI_Reduce(&isend, &irecv, 2, MPI_INTEGER, MPI_SUM, 0, MPI_COMM_WORLD);
	
	if (rank == 0) printf("irecv = [%d, %d]\n", irecv[0], irecv[1]);
	
	MPI_Finalize();

}
