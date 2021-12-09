

#include <time.h>
#include <sys/time.h> 
#include <stdlib.h> 
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

#define MICRO_SEC_IN_SEC 1000000 

int main(int argc, char **argv){
    int i;  //loop counter
    int n;

    pid_t pid;
    char *message;

    struct timeval start, end;

    printf("Initiating fork...\n");
    
    gettimeofday(&start, NULL);
    pid = fork();
    gettimeofday(&end, NULL);
    printf("\nElapsed Time: %ld micro sec\n", ((end.tv_sec * MICRO_SEC_IN_SEC + end.tv_usec) - (start.tv_sec * MICRO_SEC_IN_SEC + start.tv_usec)));

    switch(pid){
        case -1:
            perror("fork failed"); 
            exit(1);
        case 0:
            message = "This is the child";
            printf("child process: the value returned by fork is %d\n", getppid());
            n = 1;
            break;
        default:
            message = "This is the parent";
            printf("parent process PID: %d\n", pid);
            n = 1;
            break;
    }
    for(; n > 0; n--) {
        puts(message);
        sleep(1);
    }
    return 0; 
}