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

    typedef struct {
    int N; // Number of nodes
    int subMatrixSize; // Size of the sub-matrix
    } GraphInfo;

    GraphInfo graphInfo;
    int *graph = NULL;
    int Q = (int)sqrt(size);
    if (rank == 0) {
        // Read input from file specified in command line argument
        FILE *inputFile = fopen(argv[1], "r");
        if (inputFile == NULL) {
            printf("Error opening input file\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        fscanf(inputFile, "%d", &graphInfo.N);

        // Calculate the block grid size (Q) and block size
        if (Q * Q != size || graphInfo.N % Q != 0) {
            if (rank == 0) {
                printf("Error: The number of processors must be a perfect square and N must be divisible by Q where P = Q*Q.\n");
            }
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        graphInfo.subMatrixSize = graphInfo.N / Q;
        graph = (int *)malloc(graphInfo.N * graphInfo.N * sizeof(int));
        if (graph == NULL) {
            printf("Memory allocation failed for graph");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        for (int i = 0; i < graphInfo.N; i++) {
            for (int j = 0; j < graphInfo.N; j++) {
                fscanf(inputFile, "%d", &graph[i * graphInfo.N + j]);
                if (graph[i * graphInfo.N + j] == 0 && i != j) {
                    graph[i * graphInfo.N + j] = INF; // Set to INF if there is no edge
                }
            }
        }
        fclose(inputFile);
    }
    MPI_Bcast(&graphInfo, sizeof(GraphInfo), MPI_BYTE, 0, MPI_COMM_WORLD);

    int *subMatrix = (int *)malloc(graphInfo.subMatrixSize * graphInfo.subMatrixSize * sizeof(int));
    if (subMatrix == NULL) {
        printf("Memory allocation failed for subMatrix");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    // Define a submatrix data type to represent each block using MPI_Type_create_subarray
    MPI_Datatype blockType;
    int sizes[2] = {graphInfo.N, graphInfo.N};
    int subsizes[2] = {graphInfo.subMatrixSize, graphInfo.subMatrixSize};
    int starts[2] = {0, 0};
    MPI_Type_create_subarray(2, sizes, subsizes, starts, MPI_ORDER_C, MPI_INT, &blockType);
    MPI_Type_commit(&blockType);
  
    MPI_Scatter(graph, graphInfo.subMatrixSize * graphInfo.subMatrixSize, MPI_INT, subMatrix, graphInfo.subMatrixSize * graphInfo.subMatrixSize, MPI_INT, 0, MPI_COMM_WORLD);
    
    // Each process prints the matrix it received
    printf("Process %d received the following sub-matrix:\n", rank);
    printMatrix(subMatrix,graphInfo.subMatrixSize);

    MPI_Type_free(&blockType);
    // Deallocate memory
    if (rank == 0) {
        free(graph);
    }
    free(subMatrix);
    
    MPI_Finalize();
    return 0;
}
