#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define SIZE 4

int main(int argc, char *argv[]) {
  int numprocs, rank, tag, src, dest, i, blocklengths[2], displacements[2];
  float a[SIZE][SIZE] = { 1.0,  2.0,  3.0,  4.0,  5.0,  6.0,  7.0,  8.0,
                          9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 16.0};
  float b[SIZE];
  MPI_Datatype indextype;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

 if (numprocs != SIZE) {
    MPI_Finalize();
    if (rank == 0)
      printf("Run the program with %d processes!\n", SIZE);
    exit(0);
  }

  printf("Proc %d starting...\n", rank);
  tag = 0;
  src = 0;
  blocklengths[0] = 2;
  blocklengths[1] = 3;
  displacements[0] = 5;
  displacements[1] = 12;
  MPI_Type_indexed(2, blocklengths, displacements, MPI_FLOAT, &indextype);
  MPI_Type_commit(&indextype);
  if (rank == 0)
    for (i = 0; i < numprocs; i++)
      MPI_Send(a, 1, indextype, i, tag, MPI_COMM_WORLD);
  MPI_Recv(b, 5, MPI_FLOAT, src, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  printf("Proc %d received b = %3.1f %3.1f %3.1f %3.1f\n", rank, b[0], b[1], b[2], b[3]);
  
  MPI_Type_free(&indextype);
  MPI_Finalize();
}