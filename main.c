/* ************************************************************
*   @Authors    Julio López B       2021721022-2
*               Estudiante 2        Rol 2
*               Estudiante 3        Rol 3

*   @version    0.1
*************************************************************/

/*
! Cosas por hacer
TODO: Crear matriz con malloc para el hijo1 donde implementar el juego de la vida
TODO: Crear el algoritmo que implemente el juego de la vida
TODO: Añadir los mensajes de error, asserts y esas cosas (dejar para el final)

! Consideraciones:
*   En teória podríamos usar un solo puntero shm_ptr y abrir una sola vez la
*   memoria compartida, pero creo que al abrirla con cada proceso y que cada
*   uno tenga su propio puntero queda más claro que son independientes y que
*   tienen labores distintas. Al fin y al cabo son punteros así que todos
*   apuntan a lo mismo.

! Explicación de los semáforos con nombre:
*   En los códigos del profe utiliza semaforos con sem_init y sem_destroy
*   pero no me funcionaba. ChatGPT me dijo que era porque cada proceso (hijo)
*   creaba su propia copia de sem y por lo tanto al hacer sem_wait(&sem) y
*   sem_post(&sem) estos ocurrian a sus copias de sem. Así que tuve que
*   crear *sem como un puntero y darle un nombre con el cual usarlo.
*   Luego se usan sem_wait(sem) y sem_post(sem) sin el "&", pero en vez de
*   init y destroy hay que usar sem_open(), sem_close() y sem_unlink().
*   Técnicamente el semaforo es como otra memoria compartida xD
*/

#define ROWS 40
#define COLUMNS 120

#include <curses.h> //Incluir librería y compilar con la opción "-lncurses"
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
//#include <errno.h>

/*************************************************************
*   @brief      Convierte un arreglo en un número entero.
*
*   @param str  El arreglo que a convertir en número entero.
*   @return     El número entero correspondiente a el arreglo.
*   @throw      Si el arreglo no es un número entero válido.
*************************************************************/
int parse_int(char *str);




int main(int argc, char *argv[]) {

    /* Variables del problema */
    int m,i,j;

    /* Procesos */
    pid_t hijo1, hijo2;

    /* Memoria compartida */
    int shm_fd, shm_hijo1, shm_hijo2;
    void *shm_ptr, *ptr_hijo1, *ptr_hijo2;
    size_t shm_size = ROWS * COLUMNS * sizeof(char);
    const char *shm_name = "/shm-matriz-conway";

    /* Semaforos */
    sem_t *sem;
    const char *sem_name = "/sem-conway";
    // sem_t sem; // (version antigua)


    //initscr();            // Inicializar ncurses
    sem = sem_open(sem_name, O_CREAT | O_RDWR, 0666, 1); // Inicializar semaforo nombrado
    // sem_init(&sem, 1, 1);                            // Inicializar semaforo (version antigua)

    /***************************
     ** Proceso Padre (main)
    ****************************/
    /* Parte 1: Verificar entrada válida */
    if (argc != 2) {
        printf("Reintentar: %s <n>\n", argv[0]);
        printf("Donde <n> es un número entero.\n");
        exit(1);
    }
    m = parse_int(argv[1]);
    printf("Número de pasos: %d\n", m);

    /* Parte 2: Memoria compartida */
    shm_fd = shm_open(shm_name, O_CREAT | O_RDWR, S_IRWXU);         // Crear y escribir en memoria compartida
    ftruncate(shm_fd, shm_size);                                    // Configurar el tamaño del segmento de memoria compartida
    shm_ptr = mmap(0, shm_size, PROT_WRITE, MAP_SHARED, shm_fd, 0); // Mapear el segmento de memoria compartida con el puntero

    /* Parte 3: Asignar valores iniciales a la memoria compartida */
    for (i=0 ; i<ROWS*COLUMNS ; i++) {
        if (rand()%2 == 0)
            sprintf(shm_ptr+i, "%c", '1');
        else
            sprintf(shm_ptr+i, "%c", '0');
    }


    /***************************
     ** Proceso Hijo 1
    ****************************/
    hijo1 = fork();
    if (hijo1 == 0) {
        // Hijo 1 abre memoria compartida en modo READ y WRITTE
        shm_hijo1 = shm_open(shm_name, O_RDWR, S_IRUSR | S_IWUSR);
        ptr_hijo1 = mmap(0, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_hijo1, 0);

        /* Sección critica */
        sem_wait(sem);

        printf("Sección crítica del hijo 1\n");
        sleep(2);
        // Implementación de ejemplo para demostrar que recorre la memoria compartida
        printf("Lectura realizada desde Región de Memoria Compartida\n");
        char *pixel = (char*)ptr_hijo1;
        for(i=1;i<=ROWS;i++) {
            for(j=1;j<=COLUMNS;j++) {
                printf("%c",pixel[(j-1)+(i-1)*COLUMNS]);
            }
        }
        printf("\n\n");
        sem_post(sem);

        return 0;
    }


    /***************************
     ** Proceso Hijo 2
    ****************************/
    hijo2 = fork();
    if (hijo2 == 0) {
        // Hijo 2 abre memoria compartida en modo READ
        shm_hijo2 = shm_open(shm_name, O_RDONLY, S_IRUSR);
        ptr_hijo2 = mmap(0, shm_size, PROT_READ, MAP_SHARED, shm_hijo2, 0);

        /* Sección critica */
        sem_wait(sem);

        printf("Sección crítica del hijo 2\n");
        sleep(2);
        // Implementación de ejemplo para demostrar que recorre la memoria compartida
        printf("Lectura realizada desde Región de Memoria Compartida\n");
        char *pixel = (char*)ptr_hijo2;
        for(i=1;i<=ROWS;i++) {
            for(j=1;j<=COLUMNS;j++) {
                printf("%c",pixel[(j-1)+(i-1)*COLUMNS]);
            }
        }
        printf("\n\n");
        sem_post(sem);

        return 0;
    }

    // Esperar a que ambos procesos hijos terminen
    waitpid(hijo1, NULL, 0);
    waitpid(hijo2, NULL, 0);

    /***************************
     ** Proceso Padre (main)
    ****************************/
    // Desvincular la sección de memoria compartida de la memoria virtual del proceso
    munmap(shm_ptr, shm_size);
    munmap(ptr_hijo1, shm_size);
    munmap(ptr_hijo2, shm_size);

    // Cerrar la sección de memoria compartida
    close(shm_fd);
    close(shm_hijo1);
    close(shm_hijo2);

    // Eliminar la sección de memoria compartida
    if (shm_unlink(shm_name) == -1) {
        perror("shm_unlink");
        exit(1);
    }

    // sem_destroy(&sem);       // Destruir semaforo (version antigua)
    sem_close(sem);             // Cerrar semaforo
    sem_unlink(sem_name);  // Eliminar el semáforo del sistema

    //endwin();   // Cerrar ventana ncurse
    exit(0);
}


int parse_int(char *str) {
    int n;
    char *endptr;

    // Convertir el arreglo a un número entero
    n = strtol(str, &endptr, 10);

    // Verificar si el arreglo no contiene un número entero válido
    if (endptr == str || *endptr != '\0') {
        printf("Error: '%s' no es un número entero válido.\n", str);
        exit(EXIT_FAILURE);
    }

    return n;
}