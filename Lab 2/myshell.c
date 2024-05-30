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


void execute(cmdLine *cmdLinee){
    pid_t pid = fork();
    if(pid == -1){
        perror("fork error");
        exit(1);
    }
    else if(pid==0){
        if(cmdLinee->inputRedirect != NULL){
            int input = open(cmdLinee->inputRedirect, O_RDONLY);
            if (input == -1)
            {
                perror("input redirection error");
                exit(1);
            }
            if (dup2(input, 0) == -1)
            {
                perror("dup2 error");
                exit(1);
            }
            close(input);
        }

        if (cmdLinee->outputRedirect != NULL)
        {
            int output = open(cmdLinee->outputRedirect,O_WRONLY | O_APPEND);
            if (output == -1)
            {
                perror("output redirection error");
                exit(1);
            }
            if (dup2(output,1) == -1)
            {
                perror("dup2 error");
                exit(1);
            }
            close(output);
        }
        if(execvp(cmdLinee->arguments[0],cmdLinee->arguments)==-1){
            perror("execution failer");
            exit(1);
        }
    }
    else{
        if(cmdLinee->blocking == 1){
            int stat;
            waitpid(pid,&stat,0);
        }
    }
}

int main(int argc, char* argv[]){
    
int debugMode = 0;
    char input[2048];
    cmdLine *cmdLine;

    while (1) {
        char cwd[PATH_MAX];
        getcwd(cwd, sizeof(cwd));
        printf("%s$> ", cwd);

        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = '\0';
        if (strcmp(input, "quit") == 0) {
            exit(0);
        }
        if(argc > 1 && strcmp(argv[1], "-d") == 0){
            debugMode = 1;
        }
        cmdLine = parseCmdLines(input);
        if(debugMode){
            fprintf(stderr, "PID : %d\n", getpid());
            fprintf(stderr, "Executing command : %s\n", input);
        }
        if(strcmp(cmdLine->arguments[0],"suspend") == 0){
            kill(atoi(cmdLine->arguments[1]),SIGTSTP);
        }
        if(strcmp(cmdLine->arguments[0],"wake") == 0){
            kill(atoi(cmdLine->arguments[1]),SIGCONT);
        }
        if(strcmp(cmdLine->arguments[0],"kill") == 0){
            kill(atoi(cmdLine->arguments[1]),SIGTERM);
        }
        if(strcmp(cmdLine->arguments[0],"cd") == 0){
            if(chdir(cmdLine->arguments[1]) == -1){
                fprintf(stderr,"cd failed");
            }
            freeCmdLines(cmdLine);
            continue;
        }
        execute(cmdLine);
        freeCmdLines(cmdLine);
    }

    return 0;
}