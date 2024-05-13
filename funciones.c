#include "funciones.h"

int parse_int(char *str) {
    int n;
    char *endptr;
    n = strtol(str, &endptr, 10); /* Convierte el string a int */
    if (endptr == str || *endptr != '\0') { /* Verifica que no haya error */
        perror("Error al convertir a entero");
        exit(EXIT_FAILURE);
    }
    return n; /* Retorna el numero entero */
}

void asignar_valores_iniciales(void *shm_ptr) {
    /* Asigna '1' o '0' aleatoriamente a cada dirección de la memoria compartida*/
    for (int i = 0; i < ROWS * COLUMNS; i++) {
        if (rand() % 2 == 0)
            *((char *)shm_ptr + i) = '1';  
        else
            *((char *)shm_ptr + i) = '0';
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
    /* Recorrer las celdas */
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLUMNS; j++) {
            vecinos[i][j] = 0;  // Inicializar el contador de vecinos vivos en 0 para la celda actual
            /* Recorrer los vecinos */
            for (int di = -1; di <= 1; di++) {
                for (int dj = -1; dj <= 1; dj++) {
                    if (di == 0 && dj == 0)
                        continue;  // Saltar la celda actual (no es un vecino)
                    int ni = i + di, nj = j + dj;  // Calcular las coordenadas del vecino
                    // Verificar si las coordenadas del vecino están dentro de los límites de la matriz
                    if (ni >= 0 && ni < ROWS && nj >= 0 && nj < COLUMNS) {
                        // Incrementar el contador de vecinos vivos si el vecino está vivo
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
            /* Contar vecinos para la posicion (i,j) */
            int vecinos = matriz_vecinos[i][j];

            /* Ver si está viva */
            if (is_alive(shm_ptr, i, j)) {
                /* Muere por aislamiento o sobrepoblacion */
                if (vecinos < 2 || vecinos > 3) *((char *)shm_ptr + (i * COLUMNS) + j) = '0';
            } else {
                /* Nace */
                if (vecinos == 3) *((char *)shm_ptr + (i * COLUMNS) + j) = '1';
            }
        }
    }
}

void update_pantalla(void *shm_ptr) {
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLUMNS; j++) {
            move(i + 1, j + 1); /* Offset visual en ncurses */
            if (is_alive(shm_ptr, i, j)) addch(ACS_CKBOARD);  /* Visualiza la célula como un cuadro si está viva */
            else printw(" ");  /* De lo contrario, deja el espacio en blanco */
        }
    }
}

int is_alive(void *shm_ptr, int x, int y) {
    return *((char *)shm_ptr + (x * COLUMNS) + y) == '1';  
}