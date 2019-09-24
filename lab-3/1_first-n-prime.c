#include <mpi.h>
#include <stdio.h>
#include <math.h>

#define MAX 1e9

int isPrime(int n)  
{  
  // Corner cases  
  if (n <= 1)  return 0;  
  if (n <= 3)  return 1;  
  
  // This is checked so that we can skip   
  // middle five numbers in below loop  
  if (n%2 == 0 || n%3 == 0) return 0;  
  
  for (int i=5; i*i<=n; i=i+6)  
    if (n%i == 0 || n%(i+2) == 0)  
      return 0;  
  
  return 1;  
}  

int main(int argc, char **argv) {
	unsigned int num_tasks, rank, count;
	char *inmsg, *outmsg;
	MPI_Status Stat; 	// require variable for receive routines
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &num_tasks);

  // Range of each thread works
  count = (unsigned int) MAX / num_tasks;
  
	// Rank `n` will work on range from [n * count; (n + 1) * count - 1]
  int i;
  int from, to;
  from = rank * count;
  to = (rank + 1) * count - 1;
  if (rank == num_tasks - 1) to = MAX;
  for (i = from; i <= to; i++) {
    if (isPrime(i)) printf("%d ", i);
  }
  //printf("Task %d works from %d to %d\n", rank, from, to);

	MPI_Finalize();

	return 0;
}
