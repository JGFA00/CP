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
        objects[i] = (Object){type[0], i, x, y, 0, 0};
        if (strcmp(type,"FOX") == 0) {
            snprintf(ecosystem[x][y], 8, "F%d",i);
        } else if(strcmp(type,"RABBIT") == 0) {
            snprintf(ecosystem[x][y], 8, "R%d", i);
        }
        else{
            snprintf(ecosystem[x][y], 8, "X");
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

void move_rabbit(Object* rabbit, char temp_ecosystem[MAX_ROWS][MAX_COLS][8]) {
    int directions[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}}; // N, S, W, E
    bool reproduced = false;
    int new_x, new_y;
    if (rabbit->age >= GEN_PROC_RABBITS) {
        for (int i = 0; i < 4; i++) {
            new_x = rabbit->x + directions[i][0];
            new_y = rabbit->y + directions[i][1];
            if (new_x >= 0 && new_x < R && new_y >= 0 && new_y < C && temp_ecosystem[new_x][new_y][0] == '.' && ecosystem[new_x][new_y][0] != 'X') {
                // Reproduce in an empty adjacent cell
                snprintf(temp_ecosystem[new_x][new_y], 8, "R%d", num_objects);
                objects[num_objects++] = (Object){'R', num_objects, new_x, new_y, 0, 0};
                reproduced = true;
                rabbit->age = 0; // Reset age after reproduction
                break;
            }
        }
    }
    
    for (int i = 0; i < 4; i++) {
        new_x = rabbit->x + directions[i][0];
        new_y = rabbit->y + directions[i][1];
        if (new_x >= 0 && new_x < R && new_y >= 0 && new_y < C && temp_ecosystem[new_x][new_y][0] == '.' && ecosystem[new_x][new_y][0] != 'X') {
            // Ensure it's not a rock
            snprintf(temp_ecosystem[rabbit->x][rabbit->y], 8, ".");
            rabbit->x = new_x;
            rabbit->y = new_y;
            snprintf(temp_ecosystem[new_x][new_y], 8, "R%d", rabbit->id);
            break;
        }
    }
    rabbit->age++;
}

void move_fox(Object* fox, char temp_ecosystem[MAX_ROWS][MAX_COLS][8]) {
    int directions[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
    int new_x, new_y;
    bool moved = false;

    for (int i = 0; i < 4; i++) {
        new_x = fox->x + directions[i][0];
        new_y = fox->y + directions[i][1];
        if (new_x >= 0 && new_x < R && new_y >= 0 && new_y < C) {
            if (ecosystem[new_x][new_y][0] == 'R') {
                snprintf(temp_ecosystem[fox->x][fox->y], 8, ".");
                fox->x = new_x;
                fox->y = new_y;
                fox->hunger = 0; // Reset hunger after eating
                snprintf(temp_ecosystem[new_x][new_y], 8, "F%d", fox->id);
                moved = true;
                break;
            }
        }
    }

    if (!moved) {
        for (int i = 0; i < 4; i++) {
            new_x = fox->x + directions[i][0];
            new_y = fox->y + directions[i][1];
            if (new_x >= 0 && new_x < R && new_y >= 0 && new_y < C && ecosystem[new_x][new_y][0] == '.' && temp_ecosystem[new_x][new_y][0] != 'X') {
                snprintf(temp_ecosystem[fox->x][fox->y], 8, ".");
                fox->x = new_x;
                fox->y = new_y;
                snprintf(temp_ecosystem[new_x][new_y], 8, "F%d", fox->id);
                break;
            }
        }
    }

    fox->age++;
    fox->hunger++;
    if (fox->hunger >= GEN_FOOD_FOXES) {
        snprintf(temp_ecosystem[fox->x][fox->y], 8, ".");

    }
}

void simulate_generation() {
    char temp_ecosystem[MAX_ROWS][MAX_COLS][8];
    memcpy(temp_ecosystem, ecosystem, sizeof(ecosystem));

    for (int i = 0; i < num_objects; i++) {
        // Move rabbits
        if (objects[i].type == 'R') {
            move_rabbit(&objects[i], temp_ecosystem);
        }
            // Move foxes
        if (objects[i].type == 'F') {
            move_fox(&objects[i], temp_ecosystem);
        }
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
