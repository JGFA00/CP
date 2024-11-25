#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ROWS 100
#define MAX_COLS 100
#define MAX_OBJECTS 1000

typedef struct {
    char type; // 'R' for Rabbit, 'F' for Fox, 'X' for Rock
    int id; // Unique ID for each rabbit and fox
    int x;
    int y;
    int age;
    int hunger;
} Object;

int R, C, N_GEN, GEN_PROC_RABBITS, GEN_PROC_FOXES, GEN_FOOD_FOXES;
char ecosystem[MAX_ROWS][MAX_COLS][8]; // Increase size to accommodate larger IDs and rocks
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
        objects[i] = (Object){type[0], i, x, y, 0, 0};
        if (type[0] == 'X') {
            snprintf(ecosystem[x][y], 8, "*");
        } else {
            snprintf(ecosystem[x][y], 8, "%c%d", type[0], i); // Adjust buffer size to 8
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

void move_rabbit(Object* rabbit) {
    int directions[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}}; // N, S, W, E
    for (int i = 0; i < 4; i++) {
        int new_x = rabbit->x + directions[i][0];
        int new_y = rabbit->y + directions[i][1];
        if (new_x >= 0 && new_x < R && new_y >= 0 && new_y < C && ecosystem[new_x][new_y][0] == '.') {
            snprintf(ecosystem[rabbit->x][rabbit->y], 8, ".");
            rabbit->x = new_x;
            rabbit->y = new_y;
            snprintf(ecosystem[new_x][new_y], 8, "R%d", rabbit->id);
            break;
        }
    }
    rabbit->age++;
}

void move_fox(Object* fox) {
    int directions[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}}; // N, S, W, E
    for (int i = 0; i < 4; i++) {
        int new_x = fox->x + directions[i][0];
        int new_y = fox->y + directions[i][1];
        if (new_x >= 0 && new_x < R && new_y >= 0 && new_y < C) {
            if (ecosystem[new_x][new_y][0] == 'R') {
                snprintf(ecosystem[fox->x][fox->y], 8, ".");
                fox->x = new_x;
                fox->y = new_y;
                fox->hunger = 0;
                snprintf(ecosystem[new_x][new_y], 8, "F%d", fox->id);
                break;
            } else if (ecosystem[new_x][new_y][0] == '.') {
                snprintf(ecosystem[fox->x][fox->y], 8, ".");
                fox->x = new_x;
                fox->y = new_y;
                snprintf(ecosystem[new_x][new_y], 8, "F%d", fox->id);
                break;
            }
        }
    }
    fox->age++;
    fox->hunger++;
}

void simulate_generation() {
    // Move rabbits
    for (int i = 0; i < num_objects; i++) {
        if (objects[i].type == 'R') {
            move_rabbit(&objects[i]);
        }
    }

    // Move foxes
    for (int i = 0; i < num_objects; i++) {
        if (objects[i].type == 'F') {
            move_fox(&objects[i]);
            if (objects[i].hunger >= GEN_FOOD_FOXES) {
                // Fox dies of starvation
                snprintf(ecosystem[objects[i].x][objects[i].y], 8, ".");
                objects[i].type = 'D'; // Mark as dead
            }
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

    return 0;
}
