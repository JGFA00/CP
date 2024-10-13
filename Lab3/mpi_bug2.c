#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  int numprocs, rank, tag, count;
  char inmsg, outmsg;
  MPI_Status status;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  if (numprocs != 2) {
    MPI_Finalize();
    if (rank == 0)
      printf("Run the program with 2 processes!\n");
    exit(0);
  }

  printf("Proc %d starting...\n", rank);
  tag=0;
  if (rank == 0) {
    outmsg = 'x';
    MPI_Send(&outmsg, 1, MPI_CHAR, 1, tag, MPI_COMM_WORLD);
    printf("Proc %d sent char '%c'\n", rank, outmsg);
    MPI_Recv(&inmsg, 1, MPI_CHAR, 1, tag, MPI_COMM_WORLD, &status);
    printf("Proc %d received char '%c'\n", rank, inmsg);
  } else if (rank == 1) {
    outmsg = 'y';
    MPI_Recv(&inmsg, 1, MPI_CHAR, 0, tag, MPI_COMM_WORLD, &status);
    printf("Proc %d received char '%c'\n", rank, inmsg);
    MPI_Send(&outmsg, 1, MPI_CHAR, 0, tag, MPI_COMM_WORLD);
    printf("Proc %d sent char '%c'\n", rank, outmsg);
  }
  MPI_Get_count(&status, MPI_CHAR, &count);
  printf("Proc %d received %d char(s) from proc %d with tag %d\n",
	 rank, count, status.MPI_SOURCE, status.MPI_TAG);

  MPI_Finalize();
}