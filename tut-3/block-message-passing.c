#include <mpi.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv) {
	int numtasks, rank, size, count, dest, source, rc, tag = 1;
	char *inmsg, *outmsg;
	MPI_Status Stat; 	// require variable for receive routines
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	// task 0 sends to task 1 and waits to receive a return message
	if (rank == 0) {
		outmsg = "Meow"; count = 4;
		dest = 1;
		source = 1;
		//for (int i = 0; i < 3; i++) {
			MPI_Send(&outmsg, count, MPI_CHAR, dest, tag, MPI_COMM_WORLD);
			MPI_Recv(&inmsg, count, MPI_CHAR, source, tag, MPI_COMM_WORLD, &Stat);
		//}
	}

	else if (rank == 1) {
		outmsg = "Da"; count = 4;
		dest = 0;
		source = 0;
		// for (int i = 0; i < 3; i++) {
			MPI_Recv(&inmsg, count, MPI_CHAR, source, tag, MPI_COMM_WORLD, &Stat);
			MPI_Send(&outmsg, count, MPI_CHAR, dest, tag, MPI_COMM_WORLD);
		//}
	}
	
	// query reveice Stat variable and print message details
	MPI_Get_count(&Stat, MPI_CHAR, &count);
	// printf("Task %d: Receive %d char(s) \"%s\"  from task %d with tag %d \n", 
	//       rank, count, inmsg,Stat.MPI_SOURCE, Stat.MPI_TAG);
	printf("Task %d: %s.\n", rank, inmsg);
	MPI_Finalize();

	return 0;
}
