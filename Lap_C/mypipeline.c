#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>
int main() {
    int pipefd[2];
    pid_t child1, child2;

    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(1);
    }

    child1 = fork();

    if (child1 == -1) {
        perror("fork");
        exit(1);
    }

    if (child1 == 0) {
        close(STDOUT_FILENO);
        dup(pipefd[1]);
        close(pipefd[0]);
        close(pipefd[1]);
        fprintf(stderr, "(child1>redirecting stdout to the write end of the pipe...)\n");
        char* ls[] = {"ls", "-l", NULL};
        fprintf(stderr, "(child1>going to execute cmd: %s %s)\n", ls[0], ls[1]);
        execvp(ls[0], ls);
        exit(1);

    } else {

        fprintf(stderr, "(parent_process>forking...)\n");
        fprintf(stderr, "(parent_process>created process with id: %d)\n", child1);
        close(pipefd[1]);
        fprintf(stderr, "(parent_process>closing the write end of the pipe...)\n");
    }

    child2 = fork();
    if (child2 == -1) {
        perror("fork");
        exit(1);
    }

    if (child2 == 0) { // in child2 process
        close(STDIN_FILENO);
        dup(pipefd[0]);
        close(pipefd[0]);
        close(pipefd[1]);
        fprintf(stderr, "(child2>redirecting stdin to the read end of the pipe...)\n");
        char* tail[] = {"tail", "-n", "2", NULL};
        fprintf(stderr, "(child2>going to execute cmd: %s %s %s)\n", tail[0], tail[1], tail[2]);
        execvp(tail[0], tail);

        exit(EXIT_FAILURE);
    } 
    else { 
        fprintf(stderr, "(parent_process>forking...)\n");
        fprintf(stderr, "(parent_process>created process with id: %d)\n", child2);
        close(pipefd[0]);
        fprintf(stderr, "(parent_process>closing the read end of the pipe...)\n");
        fprintf(stderr, "(parent_process>waiting for child processes to terminate...)\n");
        waitpid(child1, NULL, 0);
        waitpid(child2, NULL, 0);
        fprintf(stderr, "(parent_process>exiting...)\n");
        exit(0);
    }
}
