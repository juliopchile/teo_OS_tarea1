/* ****************************************************************************
*   @Authors    Julio López         201721022-2
*               Jorge Magaña        201721071-0
*               Hector Zepeda       201921025-4

*   @version    1.0
******************************************************************************/

/*
!   Consideraciones
*   Debido al uso de múltiples procesos, cada proceso hijo tiene su propia
*   sección de memoria, es por esto que se utilizan un contador y un semaforo
*   como memorias compartidas, las cuales son destruidas al final del programa.
*   Por suerte esto no supuso problema con ncurses y el uso de forks, aunque
*   podria ser el caso en otro sistemas. Finalmente con fines de mejorar la
*   legibilidad del código, las funciones se definieron en 'funciones.c'.
*/

#include "funciones.h"

int main(int argc, char *argv[]) {
    /* ------------------------------
    ? Parte 0: Definir variables
    ------------------------------ */
    /* Variables del problema */
    int iteraciones, *count;

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
    const char *shm_name = "/shm-matriz-conway-JJH";

    /* Semaforos */
    sem_t *mutex = NULL;
    const char *mutex_name = "/mutex-conway-JJH";

    /***************************
     ** Proceso Padre (main)
     ****************************/

    /* ---------------------------------
    ? Parte 1: Verificar entrada válida
    --------------------------------- */
    if (argc != 2) {
        printf("Reintentar: %s <n>\n", argv[0]);
        printf("Donde <n> es un número entero.\n");
        exit(1);
    }
    iteraciones = parse_int(argv[1]);


    /* ------------------------------
    ? Parte 2: Memoria compartida
    ------------------------------ */
    /* Crear y escribir en memoria compartida */
    shm_fd = shm_open(shm_name, O_CREAT | O_RDWR, S_IRWXU);
    if (shm_fd == -1) {
        perror("shm_open: Padre");
        exit(EXIT_FAILURE);
    }

    /* Configurar el tamaño del segmento de memoria compartida */
    if (ftruncate(shm_fd, shm_size) == -1) {
        perror("ftruncate: Padre");
        close(shm_fd);
        exit(EXIT_FAILURE);
    }                                 

    /* Mapear el segmento de memoria compartida con el puntero */
    shm_ptr = mmap(0, shm_size, PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) {
        perror("mmap: Padre");
        close(shm_fd);
        shm_unlink(shm_name);
        exit(EXIT_FAILURE);
    }

    /* Inicializar semáforo */
    mutex = sem_open(mutex_name, O_CREAT | O_RDWR, 0666, 1);
    if (mutex == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }

    /* Inicializar contador */
    count = mmap(NULL, sizeof(*count), PROT_READ | PROT_WRITE, MAP_SHARED | 0x20, -1, 0);
    if (count == MAP_FAILED) {
        perror("mmap: count");
        exit(EXIT_FAILURE);
    }
    *count = 0;


    /* -----------------------------------------------------------
    ? Parte 3: Asignar valores iniciales a la memoria compartida
    ------------------------------------------------------------ */
    asignar_valores_iniciales(shm_ptr);

    /* Desvincular y cerrar la memoria compartida por parte del padre */
    if (munmap(shm_ptr, shm_size) == -1)    perror("munmap: Padre");
    if (close(shm_fd) == -1)                perror("close: Padre");

    initscr();  /* Inicializar ncurses */

    /***************************
     ** Proceso Hijo 1
     ****************************/
    hijo1 = fork();
    if (hijo1 < 0) {
        perror("Error al crear el proceso hijo 1");
        exit(1);
    } else if (hijo1 == 0) {
         /* Hijo 1 abre memoria compartida en modo READ y WRITTE */
        shm_hijo1 = shm_open(shm_name, O_RDWR, S_IRUSR | S_IWUSR);
        if (shm_hijo1 == -1) {
            perror("shm_open: Hijo 1");
            exit(EXIT_FAILURE);
        }
        ptr_hijo1 = mmap(0, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_hijo1, 0);
        if (ptr_hijo1 == MAP_FAILED) {
            perror("mmap: Hijo 1");
            close(shm_hijo1);
            shm_unlink(shm_name);
            exit(EXIT_FAILURE);
        }

        while (*count < iteraciones) {
            usleep(TASA_UPDATE);
            /* Sección critica */
            sem_wait(mutex);
            juego_de_la_vida(ptr_hijo1, matriz, matriz_vecinos);
            sem_post(mutex);
            /* Sección remanente */
            (*count)++;
        }
        return 0;
    }

    /***************************
     ** Proceso Hijo 2
     ****************************/
    hijo2 = fork();
    if (hijo2 < 0) {
        perror("Error al crear el proceso hijo 2");
        exit(1);
    } else if (hijo2 == 0) {
        /* Hijo 2 abre memoria compartida en modo READ */
        shm_hijo2 = shm_open(shm_name, O_RDONLY, S_IRUSR);
        if (shm_hijo2 == -1) {
            perror("shm_open: Hijo 2");
            exit(EXIT_FAILURE);
        }
        ptr_hijo2 = mmap(0, shm_size, PROT_READ, MAP_SHARED, shm_hijo2, 0);
        if (ptr_hijo2 == MAP_FAILED) {
            perror("mmap: Hijo 2");
            close(shm_hijo2);
            shm_unlink(shm_name);
            exit(EXIT_FAILURE);
        }

        while (*count < iteraciones) {
            /* Sección critica */
            sem_wait(mutex);
            update_pantalla(ptr_hijo2);
            sem_post(mutex);
            /* Sección remanente */
            refresh(); /* Refrescar la pantalla */
            usleep(TASA_REFRESCO);
        }
        return 0;
    }

    /***************************
     ** Proceso Padre (main)
     ****************************/
    /* -----------------------------------------------------------
    ? Final: Cerrar procesos, variables y memorias compartidas
    ------------------------------------------------------------ */

    /* Esperar a que ambos procesos hijos terminen */
    if (waitpid(hijo1, NULL, 0) == -1 || waitpid(hijo2, NULL, 0) == -1) perror("waitpid");

    /* Desvincular la sección de memoria compartida de la memoria virtual de los procesos hijos */
    if (munmap(ptr_hijo1, shm_size) == -1 || munmap(ptr_hijo2, shm_size) == -1) perror("munmap: Hijos");

    /* Cerrar la sección de memoria compartida */
    if (close(shm_hijo1) == -1 || close(shm_hijo2) == -1) perror("close: Hijos");

    /* Eliminar la sección de memoria compartida */
    if (shm_unlink(shm_name) == -1) {
        perror("shm_unlink");
        exit(1);
    }

    /* Cerrar semaforo */
    if (sem_close(mutex) == -1) perror("sem_close");

    /* Eliminar el semáforo del sistema */
    if (sem_unlink(mutex_name) == -1) perror("sem_unlink");

    /* Eliminar el contador del sistema */
    if (munmap(count, sizeof(*count)) == -1) perror("munmap: count");

    endwin(); /* Cerrar ventana ncurse */

    exit(EXIT_SUCCESS);
}