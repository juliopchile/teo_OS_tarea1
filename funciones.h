#ifndef FUNCIONES_H
#define FUNCIONES_H

#define ROWS 40
#define COLUMNS 120
#define TASA_UPDATE 200000  /* frecuencia actualización de cambios (microsegundos) */
#define TASA_REFRESCO 30000 /* frecuencia refresco pantalla (microsegundos) */

#include <curses.h> /* Incluir librería y compilar con la opción "-lncurses"*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <errno.h>

/******************************************************************************
 *   @brief      Convierte un arreglo en un número entero.
 *
 *   @param str   El arreglo a convertir en número entero.
 *   @return      El número entero correspondiente a el arreglo.
 *   @throw       Sale del programa si no es un número entero válido.
 *****************************************************************************/
extern int parse_int(char *str);

/******************************************************************************
 *   @brief      Asigna valores iniciales aleatorios a la memoria compartida.
 *
 *   @param shm_ptr   Puntero a la memoria compartida.
 *****************************************************************************/
extern void asignar_valores_iniciales(void *shm_ptr);

/******************************************************************************
 *   @brief      Copia los datos de la memoria compartida a una matriz.
 *
 *   @param src    Puntero a los datos en la memoria compartida.
 *   @param dest   Matriz de caracteres donde se copiarán los datos.
 *****************************************************************************/
extern void copiar_datos(void *src, char **dest);

/******************************************************************************
 *   @brief      Cuenta el número de vecinos vivos en cada celda de la matriz.
 *
 *   @param matriz    Matriz que representa el estado actual del juego de la vida.
 *   @param vecinos   Matriz de enteros donde se almacenarán los conteos de vecinos.
 *****************************************************************************/
extern void contar_vecinos(char **matriz, int **vecinos);

/******************************************************************************
 *   @brief      Actualiza el estado del juego de la vida utilizando las
 *               reglas definidas.
 *
 *   @param shm_ptr          Puntero a la memoria compartida donde se almacena
 *                           el estado del juego.
 *   @param matriz           Matriz que representa el estado actual del juego
 *                           de la vida.
 *   @param matriz_vecinos   Matriz que almacena el número de vecinos vivos
 *                           para cada celda.
 *****************************************************************************/
extern void juego_de_la_vida(void *shm_ptr, char **matriz, int **matriz_vecinos);

/******************************************************************************
 *   @brief      Actualiza la pantalla de visualización del juego de la vida.
 *
 *   @param shm_ptr   Puntero a la memoria compartida donde se almacena el
 *                    estado del juego.
 *****************************************************************************/
extern void update_pantalla(void *shm_ptr);

/******************************************************************************
 *   @brief      Verifica si una celda está viva en el estado actual del
 *               juego de la vida.
 *
 *   @param shm_ptr   Puntero a la memoria compartida donde se almacena
 *                    el estado del juego.
 *   @param x         Coordenada x de la celda.
 *   @param y         Coordenada y de la celda.
 *   @return          1 si la celda está viva, 0 en caso contrario.
 *****************************************************************************/
extern int is_alive(void *shm_ptr, int x, int y);

#endif /* FUNCIONES_H */