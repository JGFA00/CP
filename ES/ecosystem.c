#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_ROWS 100
#define MAX_COLS 100
#define MAX_OBJECTS 1000
#define MAX_STR_SIZE 16

typedef struct {
    int new_x;
    int new_y;
    bool move_requested;
} Move;

typedef struct {
    char type; // 'R' for Rabbit, 'F' for Fox, 'X' for Rock
    int id; // Unique ID for each rabbit and fox
    int x;
    int y;
    int age;
    int hunger;
    Move intended_move;
} Object;

int R, C, N_GEN, GEN_PROC_RABBITS, GEN_PROC_FOXES, GEN_FOOD_FOXES,num_objects,id_objetcs;
int directions[4][2] = {{-1, 0}, {0, 1}, {1, 0}, {0, -1}};
char ecosystem[MAX_ROWS][MAX_COLS][MAX_STR_SIZE];
Object objects[MAX_OBJECTS];
int object_index[MAX_ROWS][MAX_COLS];

void initialize_object_index() {
    for (int i = 0; i < MAX_ROWS; i++) {
        for (int j = 0; j < MAX_COLS; j++) {
            object_index[i][j] = -1; // Initialize all positions as empty
        }
    }
    // Set the initial object positions
    for (int i = 0; i < num_objects; i++) {
        object_index[objects[i].x][objects[i].y] = i;
    }
}

void read_input(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Error opening input file");
        exit(1);
    }

    fscanf(file, "%d %d %d %d %d %d %d", &GEN_PROC_RABBITS, &GEN_PROC_FOXES, &GEN_FOOD_FOXES, &N_GEN, &R, &C, &num_objects);
    id_objetcs=num_objects;

    memset(ecosystem, '.', sizeof(ecosystem));
    initialize_object_index();

    for (int i = 0; i < num_objects; i++) {
        char type[6];
        int x, y;
        fscanf(file, "%s %d %d", type, &x, &y);

        if (strcmp(type, "FOX") == 0) {
            objects[i] = (Object){'F', i, x, y, 0, 0};
            snprintf(ecosystem[x][y], MAX_STR_SIZE, "F%d", i);
        } else if (strcmp(type, "RABBIT") == 0) {
            objects[i] = (Object){'R', i, x, y, 0, 0};
            snprintf(ecosystem[x][y], MAX_STR_SIZE, "R%d", i);
        } else if (strcmp(type, "ROCK") == 0) {
            objects[i] = (Object){'X', i, x, y, 0, 0}; // Rocks don't have age or hunger
            snprintf(ecosystem[x][y], MAX_STR_SIZE, "X");
        }
        object_index[x][y] = i; // Set the initial position in the index map
    }

    fclose(file);
}

void print_ecosystem_compact() {
    printf("----------------------\n");
    for (int i = 0; i < R; i++) {
        for (int j = 0; j < C; j++) {
            printf("+------+");
        }
        printf("\n");
        for (int j = 0; j < C; j++) {
            printf("|");
            if (ecosystem[i][j][0] == '.') {
                printf("      ");
            } else if (ecosystem[i][j][0] == 'X') {
                printf(" X    ");
            } else if(ecosystem[i][j][0] == 'R' || ecosystem[i][j][0] == 'F'){
                int id;
                sscanf(ecosystem[i][j] + 1, "%d", &id);
                printf(" %c%-3d ", ecosystem[i][j][0], id);
            }
        }
        printf("|\n");
    }
    for (int j = 0; j < C; j++) {
        printf("+------+");
    }
    printf("\n----------------------\n");
}

void print_object(const Object* obj) {
    printf("\nObject Details:\n");
    printf("Type: %c\n", obj->type);
    printf("ID: %d\n", obj->id);
    printf("Position: (%d, %d)\n", obj->x, obj->y);
    printf("Age: %d\n", obj->age);
    printf("Hunger: %d\n", obj->hunger);
    printf("Intended Move: New Position (%d, %d), Move Requested: %s\n",
           obj->intended_move.new_x,
           obj->intended_move.new_y,
           obj->intended_move.move_requested ? "Yes" : "No");
}

void print_final_state() {
    printf("%d %d %d %d %d %d %d\n", GEN_PROC_RABBITS, GEN_PROC_FOXES, GEN_FOOD_FOXES, 0, R, C, num_objects);

    // Print rocks first
    for (int i = 0; i < num_objects; i++) {
        if (objects[i].type == 'X') {
            printf("ROCK %d %d\n", objects[i].x, objects[i].y);
        }
    }

    // Print rabbits next
    for (int i = 0; i < num_objects; i++) {
        if (objects[i].type == 'R') {
            printf("RABBIT %d %d\n", objects[i].x, objects[i].y);
        }
    }

    // Print foxes last
    for (int i = 0; i < num_objects; i++) {
        if (objects[i].type == 'F') {
            printf("FOX %d %d\n", objects[i].x, objects[i].y);
        }
    }
}

void cleanup_dead_objects() {
    int active_objects = 0;
    for (int i = 0; i < num_objects; i++) {
        if (objects[i].type != 'D') {
            objects[active_objects++] = objects[i];
            printf("Adding Object ID: %d, Type: %c, Position: (%d, %d)\n",objects[i].id, objects[i].type, objects[i].x, objects[i].y);
        } else {
            printf("Removing Object ID: %d, Type: %c, Position: (%d, %d)\n",
                   objects[i].id, objects[i].type, objects[i].x, objects[i].y);
            object_index[objects[i].x][objects[i].y] = -1; // Clear index
        }
    }
    num_objects = active_objects;
    printf("Cleanup complete. Remaining objects: %d\n", num_objects);
}

void collect_moves_rabbits(int gen) {
    
    for (int i = 0; i < num_objects; i++) {
        Object* obj = &objects[i];
        if (obj->type == 'R') {
            int x = obj->x;
            int y = obj->y;
            int valid_cells[4][2];
            int valid_count = 0;

            for (int j = 0; j < 4; j++) {
                int new_x = x + directions[j][0];
                int new_y = y + directions[j][1];
                if (new_x >= 0 && new_x < R && new_y >= 0 && new_y < C && ecosystem[new_x][new_y][0] == '.') {
                    valid_cells[valid_count][0] = new_x;
                    valid_cells[valid_count][1] = new_y;
                    valid_count++;
                }
            }

            if (valid_count > 0) {
                int chosen_index = (gen + x + y) % valid_count;
                obj->intended_move.new_x = valid_cells[chosen_index][0];
                obj->intended_move.new_y = valid_cells[chosen_index][1];
                obj->intended_move.move_requested = true;
            }
            print_object(obj);
        }
        
    }
}

void collect_moves_foxes(int gen) {

    for (int i = 0; i < num_objects; i++) {
        Object* obj = &objects[i];
        if (obj->type == 'F') {
            int x = obj->x;
            int y = obj->y;
            int valid_cells_eat[4][2];
            int valid_cells[4][2];
            int valid_count_eat = 0;
            int valid_count = 0;

            for (int j = 0; j < 4; j++) {
                int new_x = x + directions[j][0];
                int new_y = y + directions[j][1];
                if (new_x >= 0 && new_x < R && new_y >= 0 && new_y < C) {
                    if (ecosystem[new_x][new_y][0] == 'R') {
                        valid_cells_eat[valid_count_eat][0] = new_x;
                        valid_cells_eat[valid_count_eat][1] = new_y;
                        valid_count_eat++;
                    } else if (ecosystem[new_x][new_y][0] == '.') {
                        valid_cells[valid_count][0] = new_x;
                        valid_cells[valid_count][1] = new_y;
                        valid_count++;
                    }
                }
            }

            if (valid_count_eat > 0) {
                int chosen_index = (gen + x + y) % valid_count_eat;
                obj->intended_move.new_x = valid_cells_eat[chosen_index][0];
                obj->intended_move.new_y = valid_cells_eat[chosen_index][1];
                obj->intended_move.move_requested = true;
            } else if (valid_count > 0) {
                int chosen_index = (gen + x + y) % valid_count;
                obj->intended_move.new_x = valid_cells[chosen_index][0];
                obj->intended_move.new_y = valid_cells[chosen_index][1];
                obj->intended_move.move_requested = true;
            }
            print_object(obj);
        }
    }
}

void apply_moves() {
    char temp_ecosystem[MAX_ROWS][MAX_COLS][MAX_STR_SIZE];
    memcpy(temp_ecosystem, ecosystem, sizeof(ecosystem)); // Copy the ecosystem as a working temp version

    for (int i = 0; i < num_objects; i++) {
        Object* obj = &objects[i];
        if (obj->intended_move.move_requested) {
            int old_x = obj->x;
            int old_y = obj->y;
            int new_x = obj->intended_move.new_x;
            int new_y = obj->intended_move.new_y;

            // Update the object's position
            obj->x = new_x;
            obj->y = new_y;

            // Handle fox behavior
            if (obj->type == 'F') {
                obj->hunger++; // Increase hunger after a move

                if (ecosystem[new_x][new_y][0] == 'R') {
                    obj->hunger = 0; // Reset hunger
                    int rabbit_index = -1;
                    sscanf(temp_ecosystem[new_x][new_y] + 1, "%d", &rabbit_index);
                    objects[rabbit_index].type = 'D'; // Mark rabbit as dead
                    //snprintf(temp_ecosystem[new_x][new_y], MAX_STR_SIZE, "."); // Clear the cell
                    printf("\nFox %d ate Rabbit at (%d, %d)\n", obj->id, new_x, new_y);
                    
                }
                // Fox starves           
                if (obj->hunger >= GEN_FOOD_FOXES) {
                    obj->type = 'D'; // Mark as dead
                    snprintf(temp_ecosystem[old_x][old_y], MAX_STR_SIZE, ".");
                    printf("Fox %d starved and died at (%d, %d)\n", obj->id, new_x, new_y);
                    continue; // Skip further updates for this fox
                }
            }

            // Move the object to the new position in the temp ecosystem
            snprintf(temp_ecosystem[new_x][new_y], MAX_STR_SIZE, "%c%d", obj->type, obj->id);
            snprintf(temp_ecosystem[old_x][old_y], MAX_STR_SIZE, "."); // Clear the old position

            obj->intended_move.move_requested=false;
            // Handle rabbit procreation
            if (obj->type == 'R') {
                obj->age++;
                if (obj->age > GEN_PROC_RABBITS) { // If rabbit reaches procreation age
                    obj->age = 0; // Reset the age
                    id_objetcs++; // Increment unique ID for new rabbit
                    objects[num_objects] = (Object){'R', id_objetcs, old_x, old_y, 0, 0};
                    snprintf(temp_ecosystem[old_x][old_y], MAX_STR_SIZE, "R%d", id_objetcs);
                    printf("\nRabbit %d procreated at (%d, %d)\n", obj->id, old_x, old_y);
                    num_objects++;
                }
            }

            // Handle fox procreation
            if (obj->type == 'F') {
                obj->age++;
                if (obj->age > GEN_PROC_FOXES) { // If fox reaches procreation age
                    obj->age = 0; // Reset the age
                    id_objetcs++; // Increment unique ID for new fox
                    objects[num_objects] = (Object){'F', id_objetcs, old_x, old_y, 0, 0};
                    snprintf(temp_ecosystem[old_x][old_y], MAX_STR_SIZE, "F%d", id_objetcs);
                    printf("\nFox %d procreated at (%d, %d)\n", obj->id, old_x, old_y);
                    num_objects++;
                }
            }
        }
    }

    // Remove dead objects (rabbits and foxes) from the objects list
    cleanup_dead_objects();

    // Copy the temporary ecosystem back to the main ecosystem
    memcpy(ecosystem, temp_ecosystem, sizeof(ecosystem));
}

void apply_moves_rabbits() {
    char temp_ecosystem[MAX_ROWS][MAX_COLS][MAX_STR_SIZE];
    memcpy(temp_ecosystem, ecosystem, sizeof(ecosystem));

    for (int i = 0; i < num_objects; i++) {
        Object* obj = &objects[i];
        if (obj->type == 'R' && obj->intended_move.move_requested) {
            int old_x = obj->x;
            int old_y = obj->y;
            int new_x = obj->intended_move.new_x;
            int new_y = obj->intended_move.new_y;

            // Update positions
            object_index[old_x][old_y] = -1; // Clear old position
            object_index[new_x][new_y] = i;  // Set new position
            obj->x = new_x;
            obj->y = new_y;

            snprintf(temp_ecosystem[new_x][new_y], MAX_STR_SIZE, "R%d", obj->id);
            snprintf(temp_ecosystem[old_x][old_y], MAX_STR_SIZE, ".");
            obj->intended_move.move_requested = false;

            // Handle rabbit procreation
            obj->age++;
            if (obj->age > GEN_PROC_RABBITS) {
                obj->age = 0;
                id_objetcs++;
                objects[num_objects] = (Object){'R', id_objetcs, old_x, old_y, 0, 0};
                object_index[old_x][old_y] = num_objects; // Update object_index for new rabbit
                snprintf(temp_ecosystem[old_x][old_y], MAX_STR_SIZE, "R%d", id_objetcs);
                printf("\nRabbit %d procreated at (%d, %d)\n", obj->id, old_x, old_y);
                num_objects++;
            }
        }
    }

    memcpy(ecosystem, temp_ecosystem, sizeof(ecosystem));
}

void resolve_conflicts() {
    // Temporary array to store all conflicts by cell
    Object**** cell_conflicts;
    int conflict_counts[MAX_ROWS][MAX_COLS] = {0};
    // Allocate memory dynamically
    cell_conflicts = malloc(MAX_ROWS * sizeof(Object***));
    for (int i = 0; i < MAX_ROWS; i++) {
        cell_conflicts[i] = malloc(MAX_COLS * sizeof(Object**));
        for (int j = 0; j < MAX_COLS; j++) {
            cell_conflicts[i][j] = malloc(MAX_OBJECTS * sizeof(Object*));
        }
    }
    

    // Populate the conflict list for each cell
    for (int i = 0; i < num_objects; i++) {
        Object* obj = &objects[i];
        if (obj->intended_move.move_requested) {
            int x = obj->intended_move.new_x;
            int y = obj->intended_move.new_y;

            // Ensure coordinates are within bounds
            if (x >= 0 && x < R && y >= 0 && y < C) {
                cell_conflicts[x][y][conflict_counts[x][y]++] = obj;
            }
        }
    }

    // Resolve conflicts in each cell
    for (int x = 0; x < R; x++) {
        for (int y = 0; y < C; y++) {
            if (conflict_counts[x][y] > 1) {
                printf("Conflict detected at cell (%d, %d):\n", x, y);

                // Conflicts detected in this cell
                Object* winner = NULL;

                // Separate rabbits and foxes
                Object* rabbits[MAX_OBJECTS];
                Object* foxes[MAX_OBJECTS];
                int rabbit_count = 0, fox_count = 0;

                for (int i = 0; i < conflict_counts[x][y]; i++) {
                    Object* obj = cell_conflicts[x][y][i];
                    printf("  Object ID: %d, Type: %c, Age: %d, Hunger: %d\n", obj->id, obj->type, obj->age, obj->hunger);
                    if (obj->type == 'R') {
                        rabbits[rabbit_count++] = obj;
                    } else if (obj->type == 'F') {
                        foxes[fox_count++] = obj;
                    }
                }

                // Resolve rabbit conflicts: oldest procreation age wins
                if (rabbit_count > 0) {
                    printf("  Resolving rabbit conflict...\n");
                    winner = rabbits[0];
                    for (int i = 1; i < rabbit_count; i++) {
                        if (rabbits[i]->age > winner->age) {
                            winner = rabbits[i];
                        }
                    }
                    printf("  Winner Rabbit ID: %d, Age: %d\n", winner->id, winner->age);
                    // Mark all other rabbits as inactive
                    for (int i = 0; i < rabbit_count; i++) {
                        if (rabbits[i] != winner) {
                            rabbits[i]->type = 'D'; // Mark as dead
                            rabbits[i]->intended_move.move_requested = false;
                            printf("    Rabbit ID: %d removed.\n", rabbits[i]->id);
                        }
                    }
                }

                // Resolve fox conflicts: oldest procreation age wins
                if (fox_count > 0) {
                    printf("  Resolving fox conflict...\n");
                    winner = foxes[0];
                    for (int i = 1; i < fox_count; i++) {
                        if (foxes[i]->age > winner->age || 
                           (foxes[i]->age == winner->age && foxes[i]->hunger < winner->hunger)) {
                            winner = foxes[i];
                        }
                    }
                    printf("  Winner Fox ID: %d, Age: %d, Hunger: %d\n", winner->id, winner->age, winner->hunger);
                    // Mark all other foxes as inactive
                    for (int i = 0; i < fox_count; i++) {
                        if (foxes[i] != winner) {
                            foxes[i]->type = 'D'; // Mark as dead
                            foxes[i]->intended_move.move_requested = false;
                            printf("    Fox ID: %d removed.\n", foxes[i]->id);
                        }
                    }
                }
            }
        }
    }

    // Clean up the objects array (remove all 'D' objects)
    int active_objects = 0;
    for (int i = 0; i < num_objects; i++) {
        if (objects[i].type != 'D') {
            // Keep the active object
            objects[active_objects++] = objects[i];
        } else {
            // Log the removal of the object
            printf("Removing Object ID: %d, Type: %c, Position: (%d, %d)\n",
                   objects[i].id, objects[i].type, objects[i].x, objects[i].y);

            // Clear the object from the object_index
            object_index[objects[i].x][objects[i].y] = -1;

            // Optionally clear the ecosystem, though it's handled by apply_moves
            snprintf(ecosystem[objects[i].x][objects[i].y], MAX_STR_SIZE, ".");
        }
    }


    for (int i = 0; i < MAX_ROWS; i++) {
        for (int j = 0; j < MAX_COLS; j++) {
            free(cell_conflicts[i][j]);
        }
        free(cell_conflicts[i]);
    }
    free(cell_conflicts);
}

void apply_moves_foxes() {
    char temp_ecosystem[MAX_ROWS][MAX_COLS][MAX_STR_SIZE];
    memcpy(temp_ecosystem, ecosystem, sizeof(ecosystem));

    for (int i = 0; i < num_objects; i++) {
        Object* obj = &objects[i];
        if (obj->type == 'F' && obj->intended_move.move_requested) {
            int old_x = obj->x;
            int old_y = obj->y;
            int new_x = obj->intended_move.new_x;
            int new_y = obj->intended_move.new_y;

            // Fox eats a rabbit
            if (temp_ecosystem[new_x][new_y][0] == 'R') {
                obj->hunger = 0; // Reset hunger
                int rabbit_index = object_index[new_x][new_y]; // Get rabbit index using object_index

                if (rabbit_index != -1 && objects[rabbit_index].type == 'R') {
                    objects[rabbit_index].type = 'D'; // Mark rabbit as dead
                    object_index[new_x][new_y] = -1; // Clear the index for the rabbit
                    snprintf(temp_ecosystem[new_x][new_y], MAX_STR_SIZE, "."); // Clear the cell
                    printf("\nFox %d ate Rabbit at (%d, %d)\n", obj->id, new_x, new_y);
                }
            }

            // Update hunger and check for starvation
            obj->hunger++;
            if (obj->hunger > GEN_FOOD_FOXES) {
                obj->type = 'D'; // Mark as dead
                object_index[old_x][old_y] = -1; // Clear the index
                snprintf(temp_ecosystem[old_x][old_y], MAX_STR_SIZE, ".");
                printf("Fox %d starved and died at (%d, %d)\n", obj->id, old_x, old_y);
                continue;
            }

            // Update positions
            object_index[old_x][old_y] = -1; // Clear old position
            object_index[new_x][new_y] = i;  // Set new position
            obj->x = new_x;
            obj->y = new_y;

            snprintf(temp_ecosystem[new_x][new_y], MAX_STR_SIZE, "F%d", obj->id);
            snprintf(temp_ecosystem[old_x][old_y], MAX_STR_SIZE, ".");

            obj->intended_move.move_requested = false;

            // Handle fox procreation
            obj->age++;
            if (obj->age > GEN_PROC_FOXES) {
                obj->age = 0;
                id_objetcs++;
                objects[num_objects] = (Object){'F', id_objetcs, old_x, old_y, 0, 0};
                object_index[old_x][old_y] = num_objects; // Update object_index for new fox
                snprintf(temp_ecosystem[old_x][old_y], MAX_STR_SIZE, "F%d", id_objetcs);
                printf("\nFox %d procreated at (%d, %d)\n", obj->id, old_x, old_y);
                num_objects++;
            }
        }
    }

    cleanup_dead_objects();
    memcpy(ecosystem, temp_ecosystem, sizeof(ecosystem));
}




void simulate_generation(int gen) {
    collect_moves_rabbits(gen);
    resolve_conflicts();
    apply_moves_rabbits();
 
    collect_moves_foxes(gen);
    resolve_conflicts();
    apply_moves_foxes();
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <input_file>\n", argv[0]);
        return 1;
    }

    read_input(argv[1]);
    print_ecosystem_compact();

    for (int gen = 0; gen < N_GEN; gen++) {
        printf("Number of objects %d:\n", num_objects);
        simulate_generation(gen);
        printf("Generation %d:\n", gen + 1);
        print_ecosystem_compact();
    }

    // Print the final state in the same format as the input
    print_final_state();

    return 0;
}