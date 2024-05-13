#include "funciones.h"

int parse_int(char *str) {
    int n;
    char *endptr;
    n = strtol(str, &endptr, 10);
    if (endptr == str || *endptr != '\0') {
        printf("Error: '%s' no es un número entero válido.\n", str);
        exit(EXIT_FAILURE);
    }
    return n;
}

void asignar_valores_iniciales(void *shm_ptr) {
    for (int i = 0; i < ROWS * COLUMNS; i++) {
        if (rand() % 2 == 0) *((char *)shm_ptr + i) = '1';
        else *((char *)shm_ptr + i) = '0';
    }
}

void copiar_datos(void *src, char **dest) {
    char *src_char = (char *)src;
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLUMNS; j++) {
            dest[i][j] = src_char[i * COLUMNS + j];
        }
    }
}

void contar_vecinos(char **matriz, int **vecinos) {
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLUMNS; j++) {
            vecinos[i][j] = 0;
            for (int di = -1; di <= 1; di++) {
                for (int dj = -1; dj <= 1; dj++) {
                    if (di == 0 && dj == 0) continue;
                    int ni = i + di, nj = j + dj;
                    if (ni >= 0 && ni < ROWS && nj >= 0 && nj < COLUMNS) {
                        vecinos[i][j] += (matriz[ni][nj] == '1') ? 1 : 0;
                    }
                }
            }
        }
    }
}

void juego_de_la_vida(void *shm_ptr, char **matriz, int **matriz_vecinos) {
    copiar_datos(shm_ptr, matriz);
    contar_vecinos(matriz, matriz_vecinos);
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLUMNS; j++) {
            int vecinos = matriz_vecinos[i][j];
            if (is_alive(shm_ptr, i, j)) {
                if (vecinos < 2 || vecinos > 3) *((char *)shm_ptr + (i * COLUMNS) + j) = '0';
            }
            else {
                if (vecinos == 3) *((char *)shm_ptr + (i * COLUMNS) + j) = '1';
            }
        }
    }
}

void update_pantalla(void *shm_ptr) {
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLUMNS; j++) {
            move(i + 1, j + 1); // Offset visual en ncurses
            if (is_alive(shm_ptr, i, j)) addch(ACS_CKBOARD);
            else printw(" ");
        }
    }
    refresh();
}

int is_alive(void *shm_ptr, int x, int y) {
    return *((char *)shm_ptr + (x * COLUMNS) + y) == '1';
}
