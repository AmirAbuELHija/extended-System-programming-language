#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <signal.h>
#include <string.h>
#include "LineParser.h"
#include "linux/limits.h"

int main(int argc, char* argv[]){
    char input[100];
    int pipe_arr[2];
    if (pipe(pipe_arr) == -1) {
        perror("pipe");
        exit(1);
    }
    pid_t pid = fork();
    if (pid == -1) {
        perror("error in fork");
        exit(1);
    }
    if (pid == 0) {  
        close(pipe_arr[0]);  
        char *message = "hello";
        write(pipe_arr[1], message, 100); 
        exit(0);
    } else {  
        close(pipe_arr[1]);  
        read(pipe_arr[0], input, 100);
        printf("%s\n", input); 
        exit(0);
    }
}