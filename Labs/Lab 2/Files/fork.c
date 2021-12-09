/*      Fork Practice File      */
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
    pid_t pid;
    char *message;
    int n;
    int a = 1;
    printf("Initiating fork...\n");
    pid = fork();

    switch(pid){
        case -1:
            perror("fork failed"); 
            exit(1);
        case 0:
            fork();
            message = "This is the child";
            printf("child process: the value returned by fork is %d\n", getppid());
            n = 10;
            break;
        default:
            message = "This is the parent";
            printf("parent process PID: %d\n", pid);
            n = 10;
            break;
    }
    for(; n > 0; n--) {
        puts(message);
        sleep(1);
    }
    printf("%d", a);
    exit(0);
}