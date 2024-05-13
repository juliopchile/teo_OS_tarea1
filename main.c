/* ************************************************************
*   @Authors    Julio López B       2021721022-2
*               Estudiante 2        Rol 2
*               Estudiante 3        Rol 3

*   @version    0.7
*************************************************************/

/*
! Cosas por hacer
TODO: Hacer funcionar los semáforos para que ambos hijos puedan hacer lo suyp
TODO: Añadir los mensajes de error, asserts y esas cosas (dejar para el final)
*/

#include "funciones.h"

#define ROWS 40
#define COLUMNS 120

int main(int argc, char *argv[])
{

    /* Variables del problema */
    int n;

    /* Matriz copia */
    char **matriz = (char **)malloc(ROWS * sizeof(char *));
    for (int i = 0; i < ROWS; i++)
        matriz[i] = (char *)malloc(COLUMNS * sizeof(char));
    /* Matriz de vecinos */
    int **matriz_vecinos = (int **)malloc(ROWS * sizeof(int *));
    for (int i = 0; i < ROWS; i++)
        matriz_vecinos[i] = (int *)malloc(COLUMNS * sizeof(int));

    /* Procesos */
    pid_t hijo1 = -1, hijo2 = -1;

    /* Memoria compartida */
    int shm_fd = -1, shm_hijo1 = -1, shm_hijo2 = -1;
    void *shm_ptr = NULL, *ptr_hijo1 = NULL, *ptr_hijo2 = NULL;
    size_t shm_size = ROWS * COLUMNS * sizeof(char);
    const char *shm_name = "/shm-matriz-conway";

    /* Semaforos */
    sem_t *mutex = NULL;
    const char *mutex_name = "/mutex-conway";

    /***************************
     ** Proceso Padre (main)
     ****************************/
    /* Parte 1: Verificar entrada válida */
    if (argc != 2)
    {
        printf("Reintentar: %s <n>\n", argv[0]);
        printf("Donde <n> es un número entero.\n");
        exit(1);
    }
    n = parse_int(argv[1]);
    printf("Número de pasos: %d\n", n);

    /* Parte 2: Memoria compartida */
    shm_fd = shm_open(shm_name, O_CREAT | O_RDWR, S_IRWXU);         /* Crear y escribir en memoria compartida */
    ftruncate(shm_fd, shm_size);                                    /* Configurar el tamaño del segmento de memoria compartida */
    shm_ptr = mmap(0, shm_size, PROT_WRITE, MAP_SHARED, shm_fd, 0); /* Mapear el segmento de memoria compartida con el puntero */

    /* Parte 3: Asignar valores iniciales a la memoria compartida */
    asignar_valores_iniciales(shm_ptr);


    /* Desvincular y cerrar la memoria compartida por parte del padre */
    munmap(shm_ptr, shm_size);
    close(shm_fd);

    mutex = sem_open(mutex_name, O_CREAT | O_RDWR, 0666, 1); // Inicializar semaforo

    /***************************
     ** Proceso Hijo 1
     ****************************/
    hijo1 = fork();
    if (hijo1 == 0)
    {
         /* Hijo 1 abre memoria compartida en modo READ y WRITTE */
        shm_hijo1 = shm_open(shm_name, O_RDWR, S_IRUSR | S_IWUSR);
        ptr_hijo1 = mmap(0, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_hijo1, 0);

        /* Sección critica */

        initscr();   /* Inicializar ncurses */
        for (int i = 0; i < n; i++)
        {
            update_pantalla(ptr_hijo1);
            usleep(100000);
            juego_de_la_vida(ptr_hijo1, matriz, matriz_vecinos);
        }

        endwin();       /* Cerrar ventana ncurse */
        sem_wait(mutex);
        sem_post(mutex);

        return 0;
    }

    /***************************
     ** Proceso Hijo 2
     ****************************/
    hijo2 = fork();
    if (hijo2 == 0)
    {
        /* Hijo 2 abre memoria compartida en modo READ */
        shm_hijo2 = shm_open(shm_name, O_RDONLY, S_IRUSR);
        ptr_hijo2 = mmap(0, shm_size, PROT_READ, MAP_SHARED, shm_hijo2, 0);

        sem_wait(mutex);

        sem_post(mutex);

        return 0;
    }
    /* Esperar a que ambos procesos hijos terminen */
    waitpid(hijo1, NULL, 0);
    waitpid(hijo2, NULL, 0);

    /***************************
     ** Proceso Padre (main)
     ****************************/
    /* Desvincular la sección de memoria compartida de la memoria virtual del proceso */
    munmap(ptr_hijo1, shm_size);
    munmap(ptr_hijo2, shm_size);

    /* Cerrar la sección de memoria compartida */
    close(shm_hijo1);
    close(shm_hijo2);

    /* Eliminar la sección de memoria compartida */
    if (shm_unlink(shm_name) == -1)
    {
        perror("shm_unlink");
        exit(1);
    }

    sem_close(mutex);     /* Cerrar semaforo */
    sem_unlink(mutex_name); /* Eliminar el semáforo del sistema */
}
