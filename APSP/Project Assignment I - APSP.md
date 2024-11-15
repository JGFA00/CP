# Short Technical Report on Parallel Computation Project

## 1. Introduction and Algorithm Overview

This project implements the All-Pairs Shortest Path (APSP) problem using a parallel approach based on MPI (Message Passing Interface). The core of the implementation uses a modified Floyd-Warshall algorithm with a "Min-Plus" multiplication operation to find the shortest paths between all pairs of nodes in a graph. The focus is on improving execution times by distributing the workload across multiple processes using MPI.

### Base Idea

The base idea of the algorithm is to adapt the classical Floyd-Warshall APSP algorithm to a distributed, parallel environment using MPI. The Min-Plus matrix multiplication serves as the fundamental operation in updating path distances.

### Main Data Structures

- **GraphInfo Struct**: Contains metadata used for broadcasting, including:

  - `int N`: Number of nodes.
  - `int subMatrixSize`: Size of the sub-matrix.

- **Graph Matrix (graph)**: Represents the entire adjacency matrix of the graph, where elements store the shortest distances between nodes.

- **Local Matrices (localA, localB, newSubMatrix)**: Each process holds a submatrix of the overall graph that it operates on to calculate shortest paths in parallel.

- **Sub-Matrix Communicators (rowComm, colComm)**: MPI row and column communicators are used to ensure each process can effectively share submatrices with the appropriate neighbors during each step.

### Auxiliary Functions

- **`minPlusMultiply`**: This function performs the Min-Plus matrix multiplication on local submatrices. Each process multiplies its submatrix (`localA`) with the broadcasted submatrix (`localB`) and stores the updated values in `newSubMatrix`.
- **`initializeMatrix`**\*\* and \*\*\*\*`printMatrix`\*\*: Helper functions used for initializing matrices and printing their values for debugging purposes.

### Communication Types

- **MPI Broadcast (********`MPI_Bcast`********)**: Used to distribute submatrices (`localB`) within row communicators, allowing each process to receive the appropriate data for multiplication.
- **Scatter/Gather (********`MPI_Scatterv`********, ********`MPI_Gatherv`********)**: Used to distribute initial data blocks to processes and gather the final submatrices back to the root process.
- **Send/Receive (********`MPI_Sendrecv_replace`********)**: Facilitates the ring-like communication of matrices among columns of the grid, allowing submatrices to be shifted appropriately during each iteration.

## 2. Implementation Details

### Matrix Splitting and Distribution

The graph matrix is split into `Q x Q` blocks, where `Q = sqrt(P)` and `P` is the number of available processes. Each process is assigned a `subMatrix`, which it works on to compute the shortest paths for its respective part of the graph. This data is distributed using `MPI_Scatterv`, which handles non-contiguous memory layouts effectively.

### Communication Strategy

- During each iteration, the local submatrix (`localB`) is broadcast across the row communicators so all processes within the same row can access it.
- The submatrices are shifted using `MPI_Sendrecv_replace`, allowing all processes to have the required data for the next iteration of Min-Plus multiplication.

### Algorithm Workflow

For each iteration step, each process multiplies its assigned submatrix using Min-Plus multiplication and updates the matrix. The results are gathered back at the root process using `MPI_Gatherv` to reconstruct the final shortest path matrix.

## 3. Limitations

### Single Process Functionality

- The implementation was only able to produce correct results when running with a single process. The objective of the project was to effectively use multiple processes to achieve parallel computation, which was not successfully accomplished. Debugging and synchronization issues prevented the successful completion of the distributed version.

## 4. Difficulties and Lessons Learned

### Challenges Encountered

- **Debugging MPI Communication**: One of the main challenges was ensuring correct synchronization among processes. Issues such as deadlocks or unexpected values occurred when `MPI_Bcast` or `MPI_Sendrecv_replace` were incorrectly ordered or missing.
- **Memory Management**: Managing shared memory locations correctly without overwriting unintended values was a key difficulty, especially in ensuring `localA`, `localB`, and `newSubMatrix` were consistently and properly updated.

### Optimization Trade-offs

- The initial attempt to reuse memory by swapping pointers (`localA` and `localB`) led to incorrect results due to unintended overwrites, necessitating the use of separate memory copies to ensure correctness.
- Optimizations to minimize redundant checks within `minPlusMultiply` did improve efficiency but required careful handling to avoid missing valid updates.

### MPI Communication Complexities

- Proper setup of `MPI_Cart_create` and the related communicators was crucial for effective implementation of the Fox algorithm-like structure. Understanding how to split and broadcast correctly was key to the project’s success.

## 5. Conclusion and Suggestions

The project was only able to successfully implement a single-process version of the APSP problem using MPI, and did not achieve full parallel functionality as intended. The implementation was able to achieve reasonable speedup, attempting to demonstrate the effectiveness of distributed computing for graph algorithms, but facing significant limitations due to issues encountered in the multi-process implementation.

## 6. References

- **MPI Programming Guide**: Reference to the MPI documentation and user guides used throughout the development process.

