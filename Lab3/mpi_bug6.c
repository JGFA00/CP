#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define REQS 1000
#define DISP 100

int main(int argc, char *argv[]) {
  int numprocs, rank, tag, src, dest, num, offset, i;
  MPI_Request reqs[4 * REQS];

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
  MPI_Barrier(MPI_COMM_WORLD);
  tag = 0;
  offset = 0;
  if (rank == 0) {
    src = 1;
  } else {
    src = 0;
  }
  dest = src;
  num = 1;
  for (i = 1; i <= REQS; i++) {
    MPI_Isend(&num, 1, MPI_INT, dest, tag, MPI_COMM_WORLD, &reqs[offset]);
    MPI_Irecv(&num, 1, MPI_INT, src, tag, MPI_COMM_WORLD, &reqs[offset + 1]);
    offset += 2;
    if (i % DISP == 0)
      printf("Proc %d has done %d isends/irecvs\n", rank, i);
  }
  MPI_Waitall(2* REQS, reqs, MPI_STATUS_IGNORE);

  MPI_Finalize();
}