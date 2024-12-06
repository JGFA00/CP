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

int R, C, N_GEN, GEN_PROC_RABBITS, GEN_PROC_FOXES, GEN_FOOD_FOXES;
char ecosystem[MAX_ROWS][MAX_COLS][MAX_STR_SIZE]; // Increase size to accommodate larger IDs and rocks
Object objects[MAX_OBJECTS];
int num_objects;

void read_input(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Error opening input file");
        exit(1);
    }

    fscanf(file, "%d %d %d %d %d %d %d", &GEN_PROC_RABBITS, &GEN_PROC_FOXES, &GEN_FOOD_FOXES, &N_GEN, &R, &C, &num_objects);

    memset(ecosystem, '.', sizeof(ecosystem));

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

void print_object(const Object* obj) {
    printf("Object Type: %c\n", obj->type);
    printf("ID: %d\n", obj->id);
    printf("Position: (%d, %d)\n", obj->x, obj->y);
    printf("Age: %d\n", obj->age);
    printf("Hunger: %d\n\n", obj->hunger);
}

void move_rabbit(Object* rabbit, char temp_ecosystem[MAX_ROWS][MAX_COLS][MAX_STR_SIZE]) {
    int directions[4][2] = {{-1, 0}, {0, 1}, {1, 0}, {0, -1}}; // N, E, S, W
    int x = rabbit->x;
    int y = rabbit->y;

    // Collect valid adjacent cells
    int valid_cells[4][2];
    int valid_count = 0;
    for (int i = 0; i < 4; i++) {
        int new_x = x + directions[i][0];
        int new_y = y + directions[i][1];
        if (new_x >= 0 && new_x < R && new_y >= 0 && new_y < C && temp_ecosystem[new_x][new_y][0] == '.') {
            valid_cells[valid_count][0] = new_x;
            valid_cells[valid_count][1] = new_y;
            valid_count++;
        }
    }

    // If there are no valid cells, stay in the same place
    if (valid_count == 0) {
        return;
    }

    // Choose a cell based on the criteria
    int chosen_index = (N_GEN + x + y) % valid_count;
    int new_x = valid_cells[chosen_index][0];
    int new_y = valid_cells[chosen_index][1];

    // Move to the selected cell
    snprintf(temp_ecosystem[new_x][new_y], MAX_STR_SIZE, "R%d", rabbit->id);
    temp_ecosystem[x][y][0] = '.'; // Clear old cell
    rabbit->x = new_x;
    rabbit->y = new_y;

    // Procreation check
    rabbit->age++;
    if (rabbit->age >= GEN_PROC_RABBITS) {
        rabbit->age = 0; // Reset age
        // Leave a new rabbit in the old position
        objects[num_objects] = (Object){'R', num_objects, x, y, 0, 0};
        snprintf(temp_ecosystem[x][y], MAX_STR_SIZE, "R%d", num_objects);
        num_objects++;
    }
}



void simulate_generation() {
    char temp_ecosystem[MAX_ROWS][MAX_COLS][MAX_STR_SIZE];
    memcpy(temp_ecosystem, ecosystem, sizeof(ecosystem));

    for (int i = 0; i < num_objects; i++) {
        // Move rabbits
        if (objects[i].type == 'R') {
            move_rabbit(&objects[i], temp_ecosystem);
        }
    }
    for(int i=0;i<num_objects;i++){
        // Move foxes
/*         if (objects[i].type == 'F') {
            move_fox(&objects[i], temp_ecosystem);
        } */
    }

    memcpy(ecosystem, temp_ecosystem, sizeof(ecosystem));
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

    return 0;
}