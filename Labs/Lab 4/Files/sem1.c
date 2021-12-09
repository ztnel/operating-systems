/*FILE: sem1.c
AUTHOR: Christian Sargusingh 100970496
-------------------------------------------------------------------------------------------------------------------------------------------
DESCRIPTION: This file uses fork() to generate 2 identical processes that will attempt to access a critical section of code simultaneuously.
the binary semaphore variable will grant access to the critical section of code to one process at a time, by first changing the semaphore
variable to 0 as a process accessses the CS and then releasing the semaphore variable by setting it back to 1. Immediately a second process
can access the CS via the same methods. */


#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/sem.h>

static int set_semvalue(void);
static void del_semvalue(void);
static int semaphore_p(void);
static int semaphore_v(void);
static int sem_id;
int pid;


int main(int argc, char *argv[]) {
    int i;
    int pause_time;
    char op_char;
    srand((unsigned int)getpid());
    
    sem_id = semget((key_t)1234, 1, 0666 | IPC_CREAT);
    if (!set_semvalue()) {
        fprintf(stderr, "Failed to initialize semaphore\n"); exit(EXIT_FAILURE);
    }
    sleep(2);

    pid = fork();

    switch(pid) {  
        case -1 :
            exit(EXIT_SUCCESS);
        case 0 :
            printf("Child: %d \n", getpid());
            printf("Assigning child process id: c\n");
            op_char = 'c';      
            break;
        default :
            printf("Parent: %d \n", pid);
            printf("Assigning parent process id: p\n");
            op_char = 'p';
            break;
    }

    //CS Loop
    for(i = 0; i < 10; i++) {
        //Pull the semaphore to put it on wait state while process X taskes control
        if (!semaphore_p()) exit(EXIT_FAILURE); 

        /*----------- CRITICAL SECTION --------------*/
        printf("Entering Critical Section...\n");
        fflush(stdout); 
        pause_time = rand() % 3;
        sleep(pause_time);
        printf("%c accessing critical section \n", op_char);
        fflush(stdout);
        printf("Exiting Critical Section...\n");
        /*---------- END OF CRITICAL SECTION --------*/

        //Release the semaphore to allow another process to use 
        if (!semaphore_v()) exit(EXIT_FAILURE);
        
        pause_time = rand() % 2;
        sleep(pause_time); 
    }
    printf("\n%d - finished\n", getpid());
    if (argc > 1) { 
        sleep(10);
        del_semvalue();
    }
    exit(EXIT_SUCCESS);
}

//This method must return 0 for successful semaphore init
//This function inits semaphore using SET_VAL command in semctl call
static int set_semvalue(void) {
    union semun sem_union;
    sem_union.val = 1;
    if (semctl(sem_id, 0, SETVAL, sem_union) == -1) return(0);
    return(1);
}

//This function removes the semaphore ID using IPC_RMID command in semctl call
static void del_semvalue(void) {
    union semun sem_union;
    if (semctl(sem_id, 0, IPC_RMID, sem_union) == -1) fprintf(stderr, "Failed to delete semaphore\n");
}

//This function changes semaphore by -1 (this is the wait state)
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

//This function changes semaphore to 1 (this is the release state) so the semaphore becomes available
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