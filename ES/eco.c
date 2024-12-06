#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_ROWS 100
#define MAX_COLS 100
#define MAX_OBJECTS 1000
#define MAX_STR_SIZE 16

typedef struct {
    char type; // 'R' for Rabbit, 'F' for Fox, 'X' for Rock
    int id; // Unique ID for each rabbit and fox
    int x;
    int y;
    int age;
    int hunger;
} Object;

typedef struct {
    int new_x;
    int new_y;
    bool move_requested;
} Move;

int R, C, N_GEN, GEN_PROC_RABBITS, GEN_PROC_FOXES, GEN_FOOD_FOXES;
char ecosystem[MAX_ROWS][MAX_COLS][MAX_STR_SIZE]; // Increase size to accommodate larger IDs and rocks
Object objects[MAX_OBJECTS];
int num_objects;
Move intended_moves[MAX_OBJECTS];
int object_index[MAX_ROWS][MAX_COLS]; // Maps each position to the index of the object (-1 if empty)

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

void print_ecosystem() {
    for (int i = 0; i < R; i++) {
        for (int j = 0; j < C; j++) {
            if (ecosystem[i][j][0] == '.') {
                printf(".     ");
            } else {
                printf("%-7s", ecosystem[i][j]);
            }
        }
        printf("\n");
    }
    printf("\n");
}

void collect_moves() {
    memset(intended_moves, 0, sizeof(intended_moves));
    int directions[4][2] = {{-1, 0}, {0, 1}, {1, 0}, {0, -1}}; // N, E, S, W

    for (int i = 0; i < num_objects; i++) {
        Object* obj = &objects[i];
        int x = obj->x;
        int y = obj->y;

        if (obj->type == 'R') {
            // Rabbit movement: Choose a valid direction
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
                int chosen_index = (N_GEN + x + y) % valid_count;
                intended_moves[i].new_x = valid_cells[chosen_index][0];
                intended_moves[i].new_y = valid_cells[chosen_index][1];
                intended_moves[i].move_requested = true;
            }

        } else if (obj->type == 'F') {
            // Fox movement: First look for rabbits, then look for empty cells
            int valid_cells_eat[4][2];
            int valid_cells[4][2];
            int valid_count_eat = 0;
            int valid_count = 0;

            // Collect cells with rabbits
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

            // Prioritize eating a rabbit if possible
            if (valid_count_eat > 0) {
                int chosen_index = (N_GEN + x + y) % valid_count_eat;
                intended_moves[i].new_x = valid_cells_eat[chosen_index][0];
                intended_moves[i].new_y = valid_cells_eat[chosen_index][1];
                intended_moves[i].move_requested = true;
            } else if (valid_count > 0) {
                // Move to an empty cell if no rabbit is found
                int chosen_index = (N_GEN + x + y) % valid_count;
                intended_moves[i].new_x = valid_cells[chosen_index][0];
                intended_moves[i].new_y = valid_cells[chosen_index][1];
                intended_moves[i].move_requested = true;
            }
        }
    }
}

void resolve_conflicts() {
    // A 2D array to keep track of which object, if any, will occupy each cell
    int chosen_objects[MAX_ROWS][MAX_COLS];
    for (int i = 0; i < R; i++) {
        for (int j = 0; j < C; j++) {
            chosen_objects[i][j] = -1; // -1 means the cell is not yet assigned
        }
    }

    for (int i = 0; i < num_objects; i++) {
        if (intended_moves[i].move_requested) {
            int new_x = intended_moves[i].new_x;
            int new_y = intended_moves[i].new_y;

            if (chosen_objects[new_x][new_y] == -1) {
                chosen_objects[new_x][new_y] = i;
            } else {
                int current_occupant = chosen_objects[new_x][new_y];
                Object* current_obj = &objects[current_occupant];
                Object* new_obj = &objects[i];

                if (new_obj->type == 'R' && current_obj->type == 'R') {
                    if (new_obj->age > current_obj->age) {
                        chosen_objects[new_x][new_y] = i;
                    }
                } else if (new_obj->type == 'F' && current_obj->type == 'F') {
                    if (new_obj->age > current_obj->age) {
                        chosen_objects[new_x][new_y] = i;
                    } else if (new_obj->age == current_obj->age) {
                        if (new_obj->hunger < current_obj->hunger) {
                            chosen_objects[new_x][new_y] = i;
                        }
                    }
                }
            }
        }
    }

    for (int i = 0; i < num_objects; i++) {
        if (intended_moves[i].move_requested) {
            int new_x = intended_moves[i].new_x;
            int new_y = intended_moves[i].new_y;

            if (chosen_objects[new_x][new_y] != i) {
                intended_moves[i].move_requested = false;
            }
        }
    }
}

void apply_moves() {
    char temp_ecosystem[MAX_ROWS][MAX_COLS][MAX_STR_SIZE];
    memcpy(temp_ecosystem, ecosystem, sizeof(ecosystem));

    for (int i = 0; i < num_objects; i++) {
        if (intended_moves[i].move_requested) {
            Object* obj = &objects[i];
            int old_x = obj->x;
            int old_y = obj->y;
            int new_x = intended_moves[i].new_x;
            int new_y = intended_moves[i].new_y;

            object_index[old_x][old_y] = -1;
            object_index[new_x][new_y] = i;

            snprintf(temp_ecosystem[new_x][new_y], MAX_STR_SIZE, "%c%d", obj->type, obj->id);
            snprintf(temp_ecosystem[old_x][old_y], MAX_STR_SIZE, ".");

            obj->x = new_x;
            obj->y = new_y;

            if (obj->type == 'R') {
                obj->age++;
                if (obj->age >= GEN_PROC_RABBITS) {
                    obj->age = 0;
                }
            } else if (obj->type == 'F') {
                obj->hunger++;
                obj->age++;
                if (ecosystem[new_x][new_y][0] == 'R') {
                    obj->hunger = 0;
                    int rabbit_index = object_index[new_x][new_y];
                    if (rabbit_index != -1 && objects[rabbit_index].type == 'R') {
                        objects[rabbit_index].type = 'D';
                    }
                }
                if (obj->hunger >= GEN_FOOD_FOXES) {
                    obj->type = 'D';
                    snprintf(temp_ecosystem[new_x][new_y], MAX_STR_SIZE, ".");
                    object_index[new_x][new_y] = -1;
                }
            }
        }
    }

    // Remove dead rabbits and foxes from the objects list
    int active_objects = 0;
    for (int i = 0; i < num_objects; i++) {
        if (objects[i].type != 'D') {
            objects[active_objects++] = objects[i];
        }
    }
    num_objects = active_objects;

    memcpy(ecosystem, temp_ecosystem, sizeof(ecosystem));
}

void simulate_generation() {
    collect_moves();
    resolve_conflicts();
    apply_moves();
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

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <input_file>\n", argv[0]);
        return 1;
    }

    read_input(argv[1]);
    print_ecosystem();

    for (int gen = 0; gen < N_GEN; gen++) {
        simulate_generation();
        printf("Generation %d:\n", gen + 1);
        print_ecosystem();
    }

    // Print the final state in the same format as the input
    print_final_state();

    return 0;
}
