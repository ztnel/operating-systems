#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include <sys/sem.h>


/**



P1 and P2 take turns printing their statements

P1 has to wait until P2 finishes statement S2 
before P1 can execute statement S1

This is done by the use of semaphores

*/


//function declarations (not sure why it matters tbh)
static int init_sem(void);
static void del_sem(void);
static int sem_wait(void);
static int sem_signal(void);

//Global Variables
static int sem_id; //identification number of semaphore



//used to store the pids of PARENT and CHILD
int parent_pid, child_pid;  


/**
    main() is the part of the program to be executed
*/
int main(int argc, char *argv[]) 
{


    parent_pid = getpid();

    //in order for this program to run we pass it an arg
    if(argc == NULL){

        printf("no input argument given \n");
        exit(EXIT_FAILURE);

    }

    printf("%d \n\n",argc);

    srand((unsigned int)getpid());

    sem_id = semget((key_t)1234, 1, 0666 | IPC_CREAT);


/*              WTF IS THIS GARBAGE???????
    if (argc > 1) {
        
        //if init_sem() returns 0 - exit
        if (!init_sem()) {

            fprintf(stderr, "Failed to initialize semaphore\n");
            exit(EXIT_FAILURE);

        }

        //otherwise - go ahead and access CS 
        //op_char = ‘X’;
        
        // sleep() just slows the program a bit
        sleep(2);

    }
*/   

    if (!init_sem()) {
            fprintf(stderr, "Failed to initialize semaphore\n");
            exit(EXIT_FAILURE);
        }

    
    child_pid = fork();


    /* Conditional that both parent and child compute */
    switch(child_pid) {  

        case -1 : /* Error */
        
            break;
        
        case 0 : /* We are child */ // child_pid returned 0

            //child_pid = getpid();
            
            printf("I am a CHILD process, my pid is : %d \n", getpid());      

            break;

        default : /* We are parent */   // new_pid returned the real PID of child 

            printf("I am the PARENT  process, my pid is : %d \n", parent_pid);
            printf("My child is %d", child_pid);

            break;
            

    }
    printf("\n\n");


    for(int i = 0; i < 3; i++) {

        
        //sets the sem to wait as this program enters CS
        if(!sem_wait()) exit(EXIT_FAILURE);
        
        //--------(ENTER CRITICAL SECTION)----------

        if(!child_pid){ //if fork returned 0
            printf("!!!Child is in the section!!! \n\n");

        }
               
        if(child_pid != 0){
            printf("...Parent is in the section...\n\n");
            fflush(stdout);

        }


        //sets the sem as available
        if( !sem_signal() ) exit(EXIT_FAILURE);
        printf("The CS is now available \n\n");
        //Take a break from CS
        sleep(3);
        //--------(EXIT CRITICAL SECTION)----------



    }// end of for-loop

    exit(EXIT_SUCCESS); //end of main

}


/** 
    init_sem() initializes the semaphore using 
    the SETVAL command in a semctl() call
*/
static int init_sem()
{
    union semun sem_union;
    sem_union.val = 1;
    
    //semctl() is used
    if (semctl(sem_id, 0, SETVAL, sem_union) == -1) {
        return(0);
    }
    
    printf("Semaphore is initialized \n");

    return(1);

}


/** 
    del_sem() initializes the semaphore using 
    the SETVAL command in a semctl() call
*/
static void del_sem() 
{

    union semun sem_union;

    // semctl() is used
    if (semctl(sem_id, 0, IPC_RMID, sem_union) == -1){
        fprintf(stderr, "Failed to delete semaphore\n");
    }

    printf("Semaphore has been deleted \n");

}


/**
    sem_wait() is the wait operation: semaphore--
*/
static int sem_wait()
{

    struct sembuf sem_b;

    sem_b.sem_num = 0;
    sem_b.sem_op = -1;              /* P() */
    sem_b.sem_flg = SEM_UNDO;

    //semop() is used
    if (semop(sem_id, &sem_b, 1) == -1) {
        fprintf(stderr, "semaphore_p failed\n");
        return(0);
    }

    return(1);

}

/**
    sem_signal() is the release operation: semaphore++
*/
static int sem_signal()
{
    struct sembuf sem_b;

    sem_b.sem_num = 0;
    sem_b.sem_op = 1;           /* V() */
    sem_b.sem_flg = SEM_UNDO;
    
    //semop() is used 
    if (semop(sem_id, &sem_b, 1) == -1) {
        fprintf(stderr, "semaphore_v failed\n");
        return(0);
    }
    
    
    return(1);

}


/*The PARENT will call this when in the CS*/
void S1(){

}

/*The CHILD will call this when in the CS*/
void S2(){

}

