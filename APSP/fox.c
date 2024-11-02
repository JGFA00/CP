#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>

#define INF 99999

void printMatrix(int *matrix, int N) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (matrix[i * N + j] == INF) {
                printf("0 ");
            } else {
                printf("%d ", matrix[i * N + j]);
            }
        }
        printf("\n");
    }
}

int main(int argc, char **argv) {
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc < 2) {
        if (rank == 0) {
            printf("Usage: %s <input file>\n", argv[0]);
        }
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    int N; // Number of nodes
    int *graph = NULL;
    
    if (rank == 0) {
        // Read input from file specified in command line argument
        FILE *inputFile = fopen(argv[1], "r");
        if (inputFile == NULL) {
            printf("Error opening input file\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        fscanf(inputFile, "%d", &N);
        graph = (int *)malloc(N * N * sizeof(int));
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                fscanf(inputFile, "%d", &graph[i * N + j]);
                if (graph[i * N + j] == 0 && i != j) {
                    graph[i * N + j] = INF; // Set to INF if there is no edge
                }
            }
        }
        fclose(inputFile);
    }
    
    // Calculate the block grid size (Q) and block size
    int Q = (int)sqrt(size);
    if (Q * Q != size || N % Q != 0) {
        if (rank == 0) {
            printf("Error: The number of processors must be a perfect square and N must be divisible by Q where P = Q*Q.\n");
        }
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    int blockSize = N / Q;
    // Initial matrix  
    int *subMatrix = (int *)malloc(blockSize * blockSize * sizeof(int));

    // Create a derived datatype to describe the blocks of the matrix
    MPI_Datatype blockType;
    MPI_Type_vector(blockSize, blockSize, N, MPI_INT, &blockType);
    MPI_Type_create_resized(blockType, 0, sizeof(int), &blockType);
    MPI_Type_commit(&blockType);

    int *sendcounts = NULL;
    int *displs = NULL;
    if (rank == 0) {
        sendcounts = (int *)malloc(size * sizeof(int));
        displs = (int *)malloc(size * sizeof(int));
        for (int i = 0; i < Q; i++) {
            for (int j = 0; j < Q; j++) {
                sendcounts[i * Q + j] = 1;
                displs[i * Q + j] = i * N * blockSize + j * blockSize;
            }
        }
    }

    // Using MPI_Scatterv to handle non-contiguous sub-blocks of the matrix
    MPI_Scatterv(graph, sendcounts, displs, blockType, subMatrix, blockSize * blockSize, MPI_INT, 0, MPI_COMM_WORLD);

    // Each process prints the matrix it received
    printf("Process %d received the following sub-matrix:\n", rank);
    for (int i = 0; i < blockSize; i++) {
        for (int j = 0; j < blockSize; j++) {
            if (subMatrix[i * blockSize + j] == INF) {
                printf("0 ");
            } else {
                printf("%d ", subMatrix[i * blockSize + j]);
            }
        }
        printf("\n");
    }
    printf("\n");

    // Deallocate memory
    if (rank == 0) {
        free(graph);
        free(sendcounts);
        free(displs);
    }
    free(subMatrix);
    MPI_Type_free(&blockType);
    MPI_Finalize();
    return 0;
}
