#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  int numprocs, rank, tag, num, i;
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
  tag = 0;
  if (rank == 0) {
    for (i = 0; i < 10; i++) {
      num = i * 10;
      MPI_Send(&num, 1, MPI_INT, 1, tag, MPI_COMM_WORLD);
      printf("Proc %d sent int '%d'\n", rank, num);
    }
  }
  if (rank == 1) {
    for (i = 0; i < 10; i++) {
      MPI_Recv(&num, 1, MPI_INT, 0, tag, MPI_COMM_WORLD, &status);
      printf("Proc %d received int '%d'\n", rank, num);
    }
  }

  MPI_Finalize();
}