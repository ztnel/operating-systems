#include <unistd.h>
#include <sys/types.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/shm.h>
#include <signal.h> 
#include <time.h>
#include <sys/sem.h>
#include "matrix.h"
#include "semun.h"

//Global definitions
#define SET_SIZE 5
#define REG_SIZE 4

//Global semaphore variable
static int sem_id;
//Global flag for debug mode
bool debug;

/* MATRIX_INIT
---------------------------------------------------------------------------------------------
Description: Initializes M[] in shared memory based on user input stored by array a[]
@param: struct matrix *matrix, int a[]
@return: void */
static void matrix_init(struct matrix *matrix, int a[]){
    for (int i = 0; i < 5; i++){
        matrix->M[i] = a[i];
    }   
}


/* SWAP (SYNCHRONIZED)
---------------------------------------------------------------------------------------------
Description: This method performs the swap function for sorting in decreasing order. This
method has a chance to be synchronized if the synchronization algorithm deems that a process
requires synchronization, this method will be executed via semaphore blocking.
@param: struct matrix *matrix, int offset
@return: void */
static void swap(struct matrix *matrix, int offset){
    //set the flag to indicate swap is being performed by current process
    matrix -> F[offset] = 1;
    if (matrix -> M[offset] < matrix -> M[offset+1]){
        if (debug){
            printf("\nProcess %i: performing swap of %i and %i\n", offset, matrix -> M[offset], matrix -> M[offset+1]);
        }
        int temp;
        temp = matrix -> M[offset];
        matrix -> M[offset] = matrix -> M[offset+1];
        matrix -> M[offset+1] = temp;
    }else{
        if (debug){
            printf("\nProcess %i: no swap required\n", offset);
        }
    }
    //disable the flag to indicate swap is completed
    matrix -> F[offset] = 0;
}

/* SORTED
---------------------------------------------------------------------------------------------
Description: This method checks to see if the data set is sorted and returns true if it is 
and false otherwise.
@param: struct matrix *matrix
@return: bool */
static bool sorted(struct matrix *matrix){
    if ((matrix -> M[0] > matrix -> M[1]) &&
    (matrix -> M[1] > matrix -> M[2]) && 
    (matrix -> M[2] > matrix -> M[3]) &&
    (matrix -> M[3] > matrix -> M[4])) {
        return 1;
    }else{
        return 0;
    }
}

/* PRINT_SET
---------------------------------------------------------------------------------------------
Description: Prints data sets or flag register for user inspection. If regdata = 1 then print
the flag register contents, if regdata = 0 then print the data set registers.
@param: struct matrix *matrix, bool regdata
@return: void */
void print_set(struct matrix *matrix, bool regdata){
    if (regdata){
        printf("\nFlag Register: \n");
        for(int i = 0; i < REG_SIZE; i++){
            printf("%i ",matrix->F[i]);
        }
    }else{
        printf("\nSorted Data Set: \n");
        for(int i = 0; i < SET_SIZE; i++){
            printf("%i ",matrix->M[i]);
        }
    }
    printf("\n");
}

/* SYNCHRONIZE
---------------------------------------------------------------------------------------------
Description: Checks the flag register to deteremine if a queued swap instruction requires
synchronization or if it can operate concurrently. If an adjacent element flags are 1 (or 
accessing shared memory via the swap() function), then the process request will require 
synchronization and the function will return true . If all adjacent element flags are set to
0 then the process request does not require synchronization and the function returns false
@param: struct matrix *matrix, int offset
@return: bool */
static bool synchronize(struct matrix *matrix, int offset){
    if ((offset = 0) && (matrix -> F[offset+1] == 1)){
        return 1; 
    }else if ((matrix -> F[offset+1] == 1) || (matrix -> F[offset-1] == 1)){
        return 1;
    }else{
        return 0;
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
    //input buffer for quiet or debug mode
    char yn;
    //local integer array
    int a[SET_SIZE];
    
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

    //prompt for debug mode or quiet mode
    while(1){
        printf("Run in debug mode? [Y/N]\n");
        scanf("%s", &yn);
        if (yn == 'Y'){
            debug = true;
            break;
        }else if (yn == 'N'){
            debug = false;
            break;
        }
    }

    //request integers from user and store them in temp array a[]
    while(1){
        printf("\nInput 5 distinct integers:\n");
        for (int i = 0; i < 5; i++){
            printf("%d. ",i+1);
            scanf("%d", &a[i]);
        }
        printf("\n");
        //user check to ensure a duplicate value has been entered
        if ((a[0] == a[1]) || (a[0] == a[2]) || (a[0] == a[3]) || (a[0] == a[4])
            || (a[1] == a[2]) || (a[1] == a[3]) || (a[1] == a[4])
            || (a[2] == a[3]) || (a[2] == a[4])
            || (a[3] == a[4])){
                printf("ERROR: Input only distinct integers.\n");
        }else{
            break;
        }
    }
    

    //Initialize the input matrices with specfied test values
    matrix_init(shm, a);

    //This will create 4 child processes and leave 1 parent process remaining
    for(int i = 0; i < 4; i++){
        switch(pid = fork()) {
            case -1: /* Failure */
                perror("fork failed");
                exit(1);
            case 0: /* child */
                printf("Child Process %i: latched onto position %i and %i\n", i, i, i+1);
                //Continue checking for element swaps until the array is sorted
                while(!sorted(shm)){
                    printf("h\n");
                    //check if a blocking method is required
                    if (synchronize(shm, i)){
                        //Pull the semaphore to put it on wait state while a child process taskes control
                        if (!semaphore_p()) exit(EXIT_FAILURE);
                        //check for swap
                        swap(shm, i);
                        //Release the semaphore to allow another child process to access critical section
                        if (!semaphore_v()) exit(EXIT_FAILURE);
                    }else{  //if no blocking required then execute swap() without semaphores
                        //check for swap
                        swap(shm, i);
                    }
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

    //Print the sorted data set
    print_set(shm, 0);

    //Display the median min and max values
    printf("\nMaximum: %i\n", shm -> M[0]);
    printf("Median: %i\n", shm -> M[2]);
    printf("Minimum: %i\n", shm -> M[4]);
    
    //Notify user that shared memory is being detached and deallocated
    printf("\nDetaching and deallocating shared memory... \n");

    //Detach the shared memory segment.
    if (shmdt(shm) == -1) {
        perror("shmdt failed\n");
        exit(1);
    }
    //Deallocate the shared memory segment. 
    if (shmctl (shmid, IPC_RMID, 0) == -1) {
        perror("shmctl(IPC_RMID) failed\n");
        exit(1);
    }

    //notify of process completion and terminate main thread
    printf("\n------ Process Complete ------ \n");
    exit(0); 
}