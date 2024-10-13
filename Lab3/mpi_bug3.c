#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MSGSIZE 1000 // try with different sizes!

int main(int argc, char *argv[]) { 
  int numprocs, rank, tag, src, dest, count, i;
  char data[MSGSIZE];

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
  dest = 1;
  src = 0;
  count = 0;
  if (rank == 0) {
    for (i = 0; i < MSGSIZE; i++)
      data[i] = 'x';
    while (1) {
      MPI_Send(data, MSGSIZE, MPI_CHAR, dest, tag, MPI_COMM_WORLD);
      count ++;
      printf("Sent message number %d to proc %d...\n", count, dest);
    }
  }
  if (rank == 1) {
    while (1) {
      MPI_Recv(data, MSGSIZE, MPI_CHAR, src, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      count++;
      printf("Received message number %d from proc %d...\n", count, src);
      sleep(1);
    }
  }

  MPI_Finalize();
}
