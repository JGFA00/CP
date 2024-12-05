#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>
#include <string.h>
#include <limits.h>

#define INF INT_MAX

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

void minPlusMultiply(int *localA, int *localB, int *newSubMatrix, int subMatrixSize, int rank) {
    // Essential print to show the process involved in multiplication
    printf("Process %d performing min-plus multiplication\n", rank);

    for (int i = 0; i < subMatrixSize; i++) {
        for (int j = 0; j < subMatrixSize; j++) {
            for (int k = 0; k < subMatrixSize; k++) {
                // Check to avoid adding INF
                if (localA[i * subMatrixSize + k] != INF && localB[k * subMatrixSize + j] != INF) {
                    int newVal = localA[i * subMatrixSize + k] + localB[k * subMatrixSize + j];
                    if (newVal < newSubMatrix[i * subMatrixSize + j]) {
                        newSubMatrix[i * subMatrixSize + j] = newVal;
                    }
                }
            }
        }
    }
    // Print the result of the multiplication for debugging purposes
    //printf("Process %d after min-plus multiplication, newSubMatrix:\n", rank);
    printMatrix(newSubMatrix, subMatrixSize);
}

void initializeMatrix(int *matrix, int N) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (i == j) {
                matrix[i * N + j] = 0;
            } else {
                matrix[i * N + j] = INF;
            }
        }
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
    int *newSubMatrix = (int *)malloc(graphInfo.subMatrixSize * graphInfo.subMatrixSize * sizeof(int));
    initializeMatrix(newSubMatrix,graphInfo.subMatrixSize);  

    int source = (myRow + 1) % Q;
    int dest = (myRow + Q - 1) % Q;
    MPI_Status status;

    int count=0;
    for(int j=1;j<=graphInfo.N-1;j=j*2){
        for (int i = 0; i < graphInfo.subMatrixSize * graphInfo.subMatrixSize; i++) {
            newSubMatrix[i] = INF;
        }

        // Synchronize all processes before starting the next outer iteration
        MPI_Barrier(MPI_COMM_WORLD);

        for (int step = 0; step < graphInfo.subMatrixSize; step++) {
            int bcastRoot = step % Q;

            if (myCol == bcastRoot) {
                MPI_Bcast(localB, graphInfo.subMatrixSize * graphInfo.subMatrixSize, MPI_INT, bcastRoot, rowComm);
                // Synchronize after broadcasting to make sure all rows are updated
                MPI_Barrier(MPI_COMM_WORLD);
                memcpy(localA, localB, graphInfo.subMatrixSize * graphInfo.subMatrixSize * sizeof(int));
                minPlusMultiply(localA, localB, newSubMatrix, graphInfo.subMatrixSize, rank);
            } else {
                MPI_Bcast(localA, graphInfo.subMatrixSize * graphInfo.subMatrixSize, MPI_INT, bcastRoot, rowComm);
                // Synchronize after broadcasting to make sure all rows are updated
                MPI_Barrier(MPI_COMM_WORLD);
                minPlusMultiply(localA, localB, newSubMatrix, graphInfo.subMatrixSize, rank);
            }
            MPI_Sendrecv_replace(localB, graphInfo.subMatrixSize * graphInfo.subMatrixSize, MPI_INT, dest, 0, source, 0, colComm, &status);
        }

        // Synchronize after updating the localB to make sure all are aligned
        MPI_Barrier(MPI_COMM_WORLD);

        memcpy(localB, newSubMatrix, graphInfo.subMatrixSize * graphInfo.subMatrixSize * sizeof(int));       
        printf("count:%d\n",count);
        count++;
    }

    // Gather the sub-matrices to the root process
    MPI_Gatherv(newSubMatrix, graphInfo.subMatrixSize * graphInfo.subMatrixSize, MPI_INT, graph, sendcounts, displs, blockType, 0, MPI_COMM_WORLD);

    // Print the final result
    if (rank == 0) {
        printf("Final Shortest Path Matrix:\n");
        printMatrix(graph, graphInfo.N);
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
