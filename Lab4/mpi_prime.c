#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int N, *v;

int is_prime(int n) {
	if (n < 2) return 0;
	else return prime(n, n-1);
}

int prime(int x, int y) {
	if (y == 1) return 1;
	else if (x % y == 0) return 0;
	else return prime(x, y - 1);
}

void load_vector() {
	int i;

	N = 32000;
	v = (int *) malloc(N * sizeof(int));
	for (i = 0; i < N; i++)
	v[i] = i;
	return;
}

main(int argc, char **argv) {
	int numprocs, rank, chunk_size;
	int *local_vector = NULL;
	int *local_results = NULL;
	int *global_results = NULL;
    double start_time, end_time;
	
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	if (rank == 0) {
		load_vector();
	}

	MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);

	if (N % numprocs != 0) {
		if (rank == 0) {
			printf("N is not divisible by the number of processes.\n Run with a number of processes such that N is divisible by P.\n");
		}
		MPI_Finalize();
		exit(0);
	}

	MPI_Barrier(MPI_COMM_WORLD);
	start_time = MPI_Wtime();

	chunk_size = N / numprocs;
	local_vector = (int *) malloc(chunk_size * sizeof(int));

	MPI_Scatter(v, chunk_size, MPI_INT, local_vector, chunk_size, MPI_INT, 0, MPI_COMM_WORLD);

	local_results = (int *)malloc(chunk_size * sizeof(int));
	for (int i = 0; i < chunk_size; i++) {
		local_results[i] = is_prime(local_vector[i]);
	}

	if (rank == 0) {
		global_results = (int *) malloc(N * sizeof(int));
	}

	MPI_Gather(local_results, chunk_size, MPI_INT, global_results, chunk_size, MPI_INT, 0, MPI_COMM_WORLD);
	
	MPI_Barrier(MPI_COMM_WORLD);
	end_time = MPI_Wtime();

	
	if (rank == 0) {
		printf("Time taken: %f seconds\n", end_time - start_time);
		/* for (int i = 0; i < N; i++) {
			if (global_results[i]) {
				printf("%d is prime\n", v[i]);
			}
		} */
		free(global_results);
	}

	free(local_vector);
	free(local_results);
	if (rank == 0) {
		free(v);
	}

	MPI_Finalize();
}
