#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/shm.h>
#include <signal.h> 
#include <time.h>
#include <sys/sem.h>
#include "matrix.h"

//Global semaphore variable
static int sem_id;

/* MATRIX_INIT
---------------------------------------------------------------------------------------------
Description: Initializes input matrices in shared memory based on test specifications
@param: struct matrix
@return: void */
static void matrix_init(struct matrix *matrix){
    matrix->M[0][0] = 1;
    matrix->M[0][1] = 2;
    matrix->M[0][2] = 3;
    matrix->M[0][3] = 4;

    matrix->M[1][0] = 5;
    matrix->M[1][1] = 6;
    matrix->M[1][2] = 7;
    matrix->M[1][3] = 8;

    matrix->M[2][0] = 4;
    matrix->M[2][1] = 3;
    matrix->M[2][2] = 2;
    matrix->M[2][3] = 1;

    matrix->M[3][0] = 8;
    matrix->M[3][1] = 7;
    matrix->M[3][2] = 6;
    matrix->M[3][3] = 5;

    matrix->N[0][0] = 1;
    matrix->N[0][1] = 3;
    matrix->N[0][2] = 5;
    matrix->N[0][3] = 7;

    matrix->N[1][0] = 2;
    matrix->N[1][1] = 4;
    matrix->N[1][2] = 6;
    matrix->N[1][3] = 8;

    matrix->N[2][0] = 7;
    matrix->N[2][1] = 3;
    matrix->N[2][2] = 5;
    matrix->N[2][3] = 7;

    matrix->N[3][0] = 8;
    matrix->N[3][1] = 6;
    matrix->N[3][2] = 4;
    matrix->N[3][3] = 2;
}

/* COMPUTE_ROW
---------------------------------------------------------------------------------------------
Description: This method calculates a single row for the resulting output matrix
@param: struct matrix, int
@return: void */
static void compute_row(struct matrix *matrix, int set){
    int cumul = 0;
    for (int i = 0; i < 4; i++){
        for (int j = 0; j < 4; j++){
            cumul += matrix-> M[set][j] * matrix -> N[j][i];
        }
        matrix -> Q[set][i] = cumul;
        cumul = 0;
    }
}

/* PRINT_MATRICES
---------------------------------------------------------------------------------------------
Description: Prints matrices for user inspection
@param: struct matrix
@return: void */
static void print_matrices(struct matrix *matrix){
    printf("\nMatrix M: \n");
    for(int i = 0; i < 4; i++){
        for(int j = 0; j < 4; j++){
            printf("%i ",matrix->M[i][j]);
        }
        printf("\n");
    }
    printf("\nMatrix N: \n");
    for(int i = 0; i < 4; i++){
        for(int j = 0; j < 4; j++){
            printf("%i ",matrix->N[i][j]);
        }
        printf("\n");
    }
    printf("\nMatrix Q: \n");
    for(int i = 0; i < 4; i++){
        for(int j = 0; j < 4; j++){
            printf("%i ",matrix->Q[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

/* RESET_MEMORY
---------------------------------------------------------------------------------------------
Description: Resets output matrix Q for program rerun
@param: struct matrix
@return: void */
static void reset_memory(struct matrix *matrix){
    //traverse through each cell of matrix Q
    for (int i = 0; i < 4; i++){
        for (int j = 0; j < 4; j++){
            matrix -> Q[i][j] = 0;
        }
    }
}

/* SET_SEMVALUE
---------------------------------------------------------------------------------------------
Description: Method returns 0 for successful semaphore init. 
This function also inits semaphore using SET_VAL command in semctl call
@param: void
@return: int */
static int set_semvalue(void) {
    union semun sem_union;
    sem_union.val = 1;
    if (semctl(sem_id, 0, SETVAL, sem_union) == -1) return(0);
    return(1);
}

/* DEL_SEMVALUE
---------------------------------------------------------------------------------------------
Description: This function removes the semaphore ID using IPC_RMID command in semctl call
@param: void
@return: void */
static void del_semvalue(void) {
    union semun sem_union;
    if (semctl(sem_id, 0, IPC_RMID, sem_union) == -1) fprintf(stderr, "Failed to delete semaphore\n");
}

/* SEMAPHORE_P
---------------------------------------------------------------------------------------------
Description: This function changes semaphore by -1 (this is the wait state)
@param: void
@return: int */
static int semaphore_p(void) {
    struct sembuf sem_b;
    sem_b.sem_num = 0;
    sem_b.sem_op = -1; /* P() */ 
    sem_b.sem_flg = SEM_UNDO; 
    if (semop(sem_id, &sem_b,1) == -1) {
        fprintf(stderr, "semaphore_p failed\n");
        return(0);
    }
    return(1); 
}

/* SEMAPHORE_V
---------------------------------------------------------------------------------------------
Description: This function changes semaphore to 1 (this is the release state)
so the semaphore becomes available
@param: void
@return: int */
static int semaphore_v(void) {
    struct sembuf sem_b;
    sem_b.sem_num = 0;
    sem_b.sem_op = 1; /* V() */
    sem_b.sem_flg = SEM_UNDO;
    if (semop(sem_id, &sem_b, 1) == -1) {
        fprintf(stderr, "semaphore_v failed\n");
        return(0);
    }
    return(1); 
}

/* MAIN
---------------------------------------------------------------------------------------------
Description: Main thread
@param: void 
@return: int */
int main() {
    //initialize process ID variable
    pid_t pid;
    
    //initialize null pointer for pointing to the shared memory matrix struct
    void *matrix_memory = (void *)0;    //initializing null pointer that points to null
    struct matrix *shm;

    //Create semaphore variable
    sem_id = semget((key_t)1234, 1, 0666 | IPC_CREAT);
    if (!set_semvalue()) {
        fprintf(stderr, "Failed to initialize semaphore\n"); exit(EXIT_FAILURE);
    }

    //create shared memory construct
    int shmid;
    shmid = shmget((key_t)1234, sizeof(struct matrix), 0666 | IPC_CREAT);
    if (shmid == -1) {
        fprintf(stderr, "shmget failed\n");
        exit(EXIT_FAILURE);
    }
    
    //Attach all processes to the shared memory location
    matrix_memory = shmat(shmid, (void *)0, 0); 
    if (matrix_memory == (void *)-1) {
        fprintf(stderr, "shmat failed\n");
        exit(EXIT_FAILURE); 
    }
    //Print successful attach
    printf("Memory attached at %X\n", (int)matrix_memory);
    
    //now that the shared memory is initialized and attached point to the matrix struct
    shm = (struct matrix *)matrix_memory;
    
    //Initialize the input matrices with specfied test values
    matrix_init(shm);

    //This will create 4 child processes and leave 1 parent process remaining
    for(int i = 0; i < 4; i++){
        switch(pid = fork()) {
            case -1: /* Failure */
                perror("fork failed");
                exit(1);
            case 0: /* child */
                /*This loop uses an empty output matrix Q to assign each process to an empty row by setting the first element of a row to 1.
                Each subsequent thread will take rows that are not already flagged by another thread. This loop requires syncronization
                because multiple threads of equal priority will access the shared memory simulataneously.*/
                for (int i = 0; i < 4; i++){
                    //Pull the semaphore to put it on wait state while a child process taskes control
                    if (!semaphore_p()) exit(EXIT_FAILURE); 
                    if (shm -> Q[i][0] == 0){
                        shm -> Q[i][0] = 1;
                        printf("Child Process: working with row %i...\n", i);
                        //calculate assigned row of Q
                        compute_row(shm, i);
                        //Release the semaphore to allow another child process to access critical section
                        if (!semaphore_v()) exit(EXIT_FAILURE);
                        break;
                    }
                    //Release the semaphore to allow another child process to access critical section
                    if (!semaphore_v()) exit(EXIT_FAILURE);
                }
                //kill child process once finished
                exit(0);
        }
    }

    /*At this point we should have 1 parent executing the remaining lines*/
    //wait for all 4 child processes to finish executing
    for (int i = 4; i > 0; i--){
        wait(NULL);
    }

    //Print input and output matrices
    print_matrices(shm);
    
    //Reset Q in the shared memory struct for rerun
    printf("Resetting Q in shared memory struct... \n");
    reset_memory(shm);

    //notify of process completion and terminate main thread
    printf("----- Process Complete ------ \n");
    exit(0); 
}