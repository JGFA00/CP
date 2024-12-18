# **Parallel Ecosystem Simulation Using OpenMP – Project Report**

---

## **1. Introduction and Algorithm Overview**

This project implements a parallelized simulation of an ecosystem involving rabbits, foxes, and rocks using **OpenMP**. The goal is to simulate and observe the behavior of entities (movement, procreation, and survival) while leveraging parallel computing to optimize performance.

### **Base Idea**

The base algorithm simulates entities on a **2D grid**:
- **Rabbits** move randomly and reproduce after a certain number of steps.
- **Foxes** move to eat rabbits, increase hunger when failing to eat, and reproduce based on age.
- **Rocks** are static obstacles on the grid.

The program is parallelized using **OpenMP** to handle large numbers of entities efficiently by distributing the workload across multiple threads.

### **Main Data Structures**

- **Object Array (`objects`)**:  
   Holds all entities (Rabbits, Foxes, Rocks) and their attributes (position, age, hunger).  

- **Grid Representation (`ecosystem`)**:  
   A 2D grid to store objects' states visually for simulation output.  

- **Index Array (`object_index`)**:  
   Maps grid cells to the index of objects in the `objects` array for fast lookup.  

### **Auxiliary Functions**

- **Movement and Conflict Resolution**:
   - `collect_moves_rabbits` and `collect_moves_foxes`: Determine the intended moves of entities.  
   - `resolve_conflicts`: Resolve conflicts when multiple entities attempt to occupy the same grid cell.  

- **Entity Updates**:
   - `apply_moves_rabbits` and `apply_moves_foxes`: Update positions, manage hunger, reproduction, and remove dead entities.  

- **Synchronization Points**:
   - **Critical Sections** (`#pragma omp critical`): Ensure safe updates to shared structures like `object_index` and `ecosystem`.  

---

## **2. Implementation Details**

### **Parallel Strategy**

1. **Movement Collection**:  
   - The movement calculations for rabbits and foxes are parallelized using **OpenMP parallel for** loops.  
   - Dynamic scheduling ensures better load balancing for non-uniform workloads.  

2. **Entity Updates**:  
   - **Rabbit Procreation** and **Fox Hunger Updates** involve parallel loops with critical sections to avoid race conditions during shared grid updates.  

3. **Synchronization Points**:  
   - Shared memory updates use `#pragma omp critical` to synchronize threads.  

### **Simulation Workflow**

1. Collect moves for rabbits and foxes.  
2. Resolve conflicts to determine valid moves.  
3. Update positions, reproduce entities, and handle starvation.  
4. Repeat for a predefined number of generations (`N_GEN`).  

---

## **3. Performance Evaluation**

The simulation was tested on a **10x10 grid** with **50 entities** for **100 generations**. The performance was evaluated for **1, 2, 4, 8, and 16 threads** in the lab machine.

### **Execution Times**

| **Number of Threads** | **Execution Time (seconds)** | **Speedup**  |
|------------------------|-----------------------------|-------------:|
| 1                      | 7.473                      | 1.000x       |
| 2                      | 7.684                      | 0.973x       |
| 4                      | 7.839                      | 0.953x       |
| 8                      | 7.887                      | 0.947x       |
| 16                     | 7.831                      | 0.954x       |

---

### **Observations**

1. **Failure of Parallel Implementation**:  
   Despite efforts to parallelize the simulation, the implementation **did not achieve successful parallel execution**. The performance remained similar to or worse than the sequential version, indicating issues with synchronization, memory management, and workload distribution.  

2. **Performance Bottlenecks**:  
   - **Synchronization Overhead**:  
     The use of `#pragma omp critical` for shared updates (e.g., `object_index` and `ecosystem`) led to significant contention, preventing threads from executing efficiently in parallel.  

   - **Cache Contention and False Sharing**:  
     Multiple threads writing to closely located memory addresses (e.g., shared arrays) caused cache invalidations, reducing the efficiency of the parallel execution.  

   - **Workload Imbalance**:  
     Tasks such as movement collection (`collect_moves_rabbits` and `collect_moves_foxes`) did not distribute evenly among threads, leading to idle threads in sparse grid regions.  

---

## **4. Conclusion**

The parallel implementation of the ecosystem simulation was **unsuccessful** due to issues with thread synchronization, race conditions, and workload distribution. Key challenges included significant overhead caused by critical sections, cache contention, and unbalanced task execution.

To achieve a correct and efficient parallel implementation, further debugging and improvements are required:
1. **Synchronization**: Replace critical sections with finer-grained synchronization (e.g., atomic operations) to reduce contention.  
2. **Memory Management**: Optimize data structures to avoid cache conflicts and false sharing.  
3. **Load Balancing**: Refine task distribution strategies to ensure even workload across threads.  

Additionally, the implementation was carefully designed to **allow compilation with or without OpenMP support**. By using conditional compilation, the code remains flexible and can run as a sequential version when compiled without the OpenMP flag (`-fopenmp`).

While the intended parallel behavior was not achieved, this project provided valuable insights into shared memory programming and the complexities of OpenMP parallelization. These lessons will be instrumental in addressing similar challenges in future implementations.

---

