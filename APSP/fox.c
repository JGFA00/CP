#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>
#include <string.h>

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

    // Define a submatrix data type to represent each block using MPI_Type_create_subarray
    MPI_Datatype blockType;
    MPI_Type_vector(graphInfo.subMatrixSize, graphInfo.subMatrixSize, graphInfo.N, MPI_INT, &blockType);
    MPI_Type_create_resized(blockType, 0, sizeof(int), &blockType);
    MPI_Type_commit(&blockType);
    
    int *sendcounts = NULL;
    int *displs = NULL;
    if (rank == 0) {
        sendcounts = (int *)malloc(size * sizeof(int));
        displs = (int *)malloc(size * sizeof(int));
         for (int i = 0; i < Q; i++) {
            for (int j = 0; j < Q; j++) {
                int idx = i * Q + j;
                sendcounts[idx] = 1; // Each sub-matrix is considered a "block"
                displs[idx] = i * graphInfo.N * graphInfo.subMatrixSize + j * graphInfo.subMatrixSize;
            }
        }
    }

    // Using MPI_Scatterv to handle non-contiguous sub-blocks of the matrix
    MPI_Scatterv(graph, sendcounts, displs, blockType, subMatrix, graphInfo.subMatrixSize * graphInfo.subMatrixSize, MPI_INT, 0, MPI_COMM_WORLD);

    // Each process prints the matrix it received
    printf("Process %d received:\n", rank);
    printMatrix(subMatrix,graphInfo.subMatrixSize);


    // Set up the grid communicators using MPI_Cart_create
    MPI_Comm gridComm, rowComm, colComm;
    int dims[2] = {Q, Q}; // Dimension of the grid
    int periods[2] = {1, 1}; // Make the grid periodic
    MPI_Cart_create(MPI_COMM_WORLD, 2, dims, periods, 1, &gridComm);

    int coords[2];
    int myRow, myCol;
    MPI_Cart_coords(gridComm, rank, 2, coords);
    myRow = coords[0];
    myCol = coords[1];

    // Create row and column communicators
    MPI_Comm_split(gridComm, myRow, myCol, &rowComm);
    MPI_Comm_split(gridComm, myCol, myRow, &colComm);

    int *localA = (int *)malloc(graphInfo.subMatrixSize * graphInfo.subMatrixSize * sizeof(int));
    int *localB = subMatrix; // subMatrix initially represents local B

    for (int step = 0; step < Q; step++) {
        int bcastRoot = (myRow + step) % Q;

        if (myCol == bcastRoot) {
            // Broadcast local B to the entire row
            for (int row = 0; row < graphInfo.subMatrixSize; row++) {
                MPI_Bcast(&localB[row * graphInfo.subMatrixSize], graphInfo.subMatrixSize, MPI_INT, bcastRoot, rowComm);
            }
            // Copy localB to localA to use for multiplication
            memcpy(localA, localB, graphInfo.subMatrixSize * graphInfo.subMatrixSize * sizeof(int));
        } else {
            // Receive broadcasted data
            for (int row = 0; row < graphInfo.subMatrixSize; row++) {
                MPI_Bcast(&localA[row * graphInfo.subMatrixSize], graphInfo.subMatrixSize, MPI_INT, bcastRoot, rowComm);
            }
        }

        if (rank == 1) {
            for (int row = 0; row < graphInfo.subMatrixSize; row++) {
                printf("Process %d received row %d after step %d: ", rank, row, step);
                for (int col = 0; col < graphInfo.subMatrixSize; col++) {
                    if(localA[row * graphInfo.subMatrixSize + col] !=INF)
                        printf("%d ", localA[row * graphInfo.subMatrixSize + col]);
                    else
                        printf("0 ");
                }
                printf("\n");
            }
        }
    }

    MPI_Type_free(&blockType);
    MPI_Comm_free(&colComm);
    MPI_Comm_free(&rowComm);
    // Deallocate memory
    if (rank == 0) {
        free(graph);
        free(sendcounts);
        free(displs);
    }
    free(subMatrix);
    free(localA);
    
    MPI_Finalize();
    return 0;
}
