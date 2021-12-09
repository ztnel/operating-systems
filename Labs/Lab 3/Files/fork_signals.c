#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/shm.h>
#include <signal.h> 
#include <time.h>
#include "shm.h"

static int alarm_fired = 0;
const int THRESHOLD = 10;


void ding(int sig) {
    alarm_fired = 1;
}

int main() {
    void *shared_memory = (void *)0;
    struct shared_use_st *shared_stuff;
    int shmid;
    int r;

    shmid = shmget((key_t)1234, sizeof(struct shared_use_st), 0666 | IPC_CREAT);
    if (shmid == -1) {
        fprintf(stderr, "shmget failed\n");
        exit(EXIT_FAILURE);
    }

    shared_memory = shmat(shmid, (void *)0, 0); 
    if (shared_memory == (void *)-1) {
        fprintf(stderr, "shmat failed\n");
        exit(EXIT_FAILURE); 
    }

    printf("Memory attached at %X\n", (int)shared_memory);
    shared_stuff = (struct shared_use_st *)shared_memory; 
    
    pid_t pid;
    printf("Initiating fork_signals.c\n");
    pid = fork(); 
    switch(pid) {
        case -1:
            /* Failure */ 
            perror("fork failed");
            exit(1);
        case 0:
            /* child */
            while(1) {
                sleep(1);
                printf("Generating random number...\n");
                r = rand() % 20;
                printf("Sending %i to shm...\n", r);
                shared_stuff -> rnd = r;
                if (r > THRESHOLD){
                    kill(getppid(), SIGALRM);
                }
            }      
    }

    /* if we get here we are the parent process */ 
    printf("Waiting for threshold to be reached...\n"); 
    
    struct sigaction act;
    act.sa_handler = ding;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGALRM, &act, 0);

    while (1){
        pause();
        if (alarm_fired){
            shared_stuff = (struct shared_use_st *)shared_memory;
            printf("Current memory: %i\n", shared_stuff->rnd);
            sleep(rand() % 4); 
        }
    }
    
    printf("done\n");
    exit(0); 
}