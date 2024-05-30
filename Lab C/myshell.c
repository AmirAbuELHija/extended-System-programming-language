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

#define TERMINATED -1
#define RUNNING 1
#define SUSPENDED 0
#define HISTLEN 21

char *history[HISTLEN] = {NULL};
int newest = 0;
int oldest = 0;

typedef struct process
{
    cmdLine *cmd;
    pid_t pid;
    int status;
    struct process *next;
} process;

process *process_list = NULL;

void deleteProcess(process **process_list, int pid)
{
    process *prev_process = NULL;
    process *curr_process = *process_list;

    while (curr_process != NULL && curr_process->pid != pid)
    {
        prev_process = curr_process;
        curr_process = curr_process->next;
    }

    if (curr_process != NULL)
    {
        if (prev_process != NULL)
        {
            prev_process->next = curr_process->next;
        }
        else
        {

            *process_list = curr_process->next;
        }
        freeCmdLines(curr_process->cmd);
        free(curr_process);
    }
}

void updateProcessStatus(process *process_list, int pid, int status) {
    process *curr = process_list;
    while (curr != NULL) {

        if (curr->pid == pid) {
            curr->status = status;
            break;
        }
        curr = curr->next;
    }
}


void updateProcessList(process **process_list){
    process* p = *process_list;
    while(p != NULL){
        pid_t pid = p->pid;
        int status = p->status;
        int wait_ret;
        int exit_status;
        if (status == RUNNING) {
            wait_ret = waitpid(pid, &exit_status, WNOHANG);
            if (wait_ret == -1) {
                updateProcessStatus(p, pid, TERMINATED);
            } else if (wait_ret == pid) {
                if (WIFEXITED(exit_status)) {
                    updateProcessStatus(p, pid, TERMINATED);
                } else if (WIFSIGNALED(exit_status)) {
                    updateProcessStatus(p, pid, TERMINATED);

                } else if (WIFSTOPPED(exit_status)) {
                    updateProcessStatus(p, pid, SUSPENDED);

                } else if (WIFCONTINUED(exit_status)) {
                    updateProcessStatus(p, pid, RUNNING);
                }
            }
        }
        p = p->next;
    }
}

void addProcess(process** process_list, cmdLine* cmd, pid_t pid) {
    process *proc = (process*) malloc(sizeof(process));
    proc->cmd = cmd;
    proc->pid = pid;
    proc->status = RUNNING;
    proc->next = NULL;

    if (*process_list == NULL) {
        *process_list = proc;
    } else {
        process *curr_proc = *process_list;
        while (curr_proc->next != NULL) {
            curr_proc = curr_proc->next;
        }
        curr_proc->next = proc;
    }
}

void printProcessList(process** process_list) {
    updateProcessList(process_list);
    
    printf("PID\tCommand\t\tSTATUS\n");
    process* curr = *process_list;
    process* prev = NULL;
    while (curr != NULL) {
        pid_t pid = curr->pid;
        int status = curr->status;
        printf("%d\t%s\t\t", pid, curr->cmd->arguments[0]);
        if (status == TERMINATED) {
            printf("Terminated\n");
            
            if (prev == NULL) {
                *process_list = curr->next;
            } else {
                prev->next = curr->next;
            }
            freeCmdLines(curr->cmd);
            free(curr);
            curr = (prev == NULL ? *process_list : prev->next);
        } else if (status == RUNNING) {
            printf("Running\n");
            prev = curr;
            curr = curr->next;
        } else {
            printf("Suspended\n");
            prev = curr;
            curr = curr->next;
        }
    }
}

void freeProcessList(process *process_list)
{
    process *curr = process_list;
    while (curr != NULL)
    {
        process *next = curr->next;
        freeCmdLines(curr->cmd);
        free(curr);
        curr = next;
    }
}

void execute(cmdLine (*cmdLinee)) {
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork error");
        exit(1);
    } else if (pid == 0) {
        if (cmdLinee->inputRedirect != NULL) {
            int input = open(cmdLinee->inputRedirect, O_RDONLY);
            if (input == -1) {
                perror("input redirection error");
                exit(1);
            }
            if (dup2(input, 0) == -1) {
                perror("dup2 error");
                exit(1);
            }
            close(input);
        }

        if (cmdLinee->outputRedirect != NULL) {
            int output = open(cmdLinee->outputRedirect, O_WRONLY | O_APPEND);
            if (output == -1) {
                perror("output redirection error");
                exit(1);
            }
            if (dup2(output, 1) == -1) {
                perror("dup2 error");
                exit(1);
            }
            close(output);
        }
        if (execvp(cmdLinee->arguments[0], cmdLinee->arguments) == -1) {
            perror("execution failure");
            exit(1);
        }
    } else {
        addProcess(&process_list, (cmdLine *) cmdLinee, pid);
        if (cmdLinee->blocking == 1) {
            int stat;
            if (waitpid(pid, &stat, 0) == -1) {
                perror("waitpid error");
            }
        }
    }
}

void execute2(cmdLine *cmdLinee,cmdLine *cmdLinee2){
    int pipefd[2];
    pid_t child1, child2;

    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    child1 = fork();

    if (child1 == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (child1 == 0) {
        close(STDOUT_FILENO);
        dup(pipefd[1]);
        close(pipefd[0]);
        if (cmdLinee->inputRedirect != NULL) {
            int input = open(cmdLinee->inputRedirect, O_RDONLY);
            if (input == -1) {
                perror("input redirection error");
                exit(1);
            }
            if (dup2(input, 0) == -1) {
                perror("dup2 error");
                exit(1);
            }
            close(input);
        }

        if (cmdLinee->outputRedirect != NULL) {
          perror("pipe activated");
          exit(1);
        }
        if (execvp(cmdLinee->arguments[0], cmdLinee->arguments) == -1) {
            perror("execution failure");
            exit(1);
        }

    } else {
        close(pipefd[1]);
        addProcess(&process_list,  cmdLinee, child1);
    }

    child2 = fork();

    if (child2 == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (child2 == 0) { // in child2 process
        close(STDIN_FILENO);
        dup(pipefd[0]);
        close(pipefd[0]);
        close(pipefd[1]);
        if (cmdLinee2->inputRedirect != NULL) {
            perror("pipe activated");
            exit(1);
        }
        if (cmdLinee2->outputRedirect != NULL) {
            int output = open(cmdLinee2->outputRedirect, O_WRONLY | O_APPEND);
            if (output == -1) {
                perror("output redirection error");
                exit(1);
            }
            if (dup2(output, 1) == -1) {
                perror("dup2 error");
                exit(1);
            }
            close(output);
        }
        if (execvp(cmdLinee2->arguments[0], cmdLinee2->arguments) == -1) {
            perror("execution failure");
            exit(1);
        }

    } else { // in parent process
        close(pipefd[0]);
        addProcess(&process_list, cmdLinee2, child2);
        addProcess(&process_list,  cmdLinee, child1);
        waitpid(child1, NULL, 0);
        waitpid(child2, NULL, 0);
    }
}

void print_history()
{
    int i = oldest;
    int count = 1;
    while (i != newest)
    {
        if (history[i] != NULL)
        {
            printf("%d: %s\n", count, history[i]);
            count++;
        }
        i = (i + 1) % HISTLEN;
    }
}

void handler(int sig)
{
    printf("\nRecieved Signal : %s\n", strsignal(sig));
    if (sig == SIGTSTP)
    {
        signal(SIGTSTP, SIG_DFL);
    }
    else if (sig == SIGCONT)
    {
        signal(SIGCONT, SIG_DFL);
    }
    signal(sig, SIG_DFL);
    raise(sig);
}

void free_history(){
    int i = oldest;
    int count = 1;
    while (i != newest)
    {
        if (history[i] != NULL)
        {
            free(history[i]);
            count++;
        }
        i = (i + 1) % HISTLEN;
    }

}

int main(int argc, char *argv[])
{

    int debugMode = 0;
    char command[2048];
    cmdLine *cmdLine;
    if (argc > 1 && strcmp(argv[1], "-d") == 0)
    {
        debugMode = 1;
    }
    while (1)
    {
        char cwd[PATH_MAX];
        getcwd(cwd, sizeof(cwd));
        printf("%s$ ", cwd);

        fgets(command, sizeof(command), stdin);
        if (strcmp(command, "\n") == 0)
        {
            continue;
        }
        
        command[strcspn(command, "\n")] = '\0';

        if (strcmp(command, "!!") == 0)
        {
            if (newest == oldest)
            {
                fprintf(stderr, "No commands in history\n");
            }
            else
            {
                strcpy(command, history[(newest - 1 + HISTLEN) % HISTLEN]);
                printf("%s\n", command);
            }
        }

        if (command[0] == '!')
        {
            int index = atoi(&command[1]);
            if (index < 1 && index > 20)
            {
                fprintf(stderr, "out of range\n");
                continue;
            }
            char *old_command = history[index - 1];
            if (old_command == NULL)
            {
                fprintf(stderr, "No such command in history\n");
                continue;
            }
            strcpy(command, old_command);
            printf("%s\n", command);
        }

        if (command != NULL)
        {
            char *new_command = (char *)malloc(strlen(command) + 1);
            strcpy(new_command, command);
            history[newest] = new_command;
            newest = (newest + 1) % HISTLEN;
            if (newest == oldest)
            {

                free(history[oldest]);
                oldest = (oldest + 1) % HISTLEN;
            }
        }
        if (command != NULL)
        {
            if (strcmp(command, "quit") == 0)
            {
                break;
            }
            if (strcmp(command, "history") == 0)
            {
                print_history();
                continue;
            }
            if (strcmp(command, "procs") == 0)
            {
                printProcessList(&process_list);
                continue;
            }
            
            cmdLine = parseCmdLines(command);
            
            if (debugMode)
            {
                fprintf(stderr, "PID : %d\n", getpid());
                fprintf(stderr, "Executing command : %s\n", command);
            }

            if (strcmp(cmdLine->arguments[0], "cd") == 0)
            {
                if (chdir(cmdLine->arguments[1]) == -1)
                {
                    fprintf(stderr, "cd failed");
                }
                freeCmdLines(cmdLine);
                continue;
            }
            
            else if (cmdLine->next != NULL)
            {
                execute2(cmdLine, (cmdLine->next));
                continue;
            }

            if (strcmp(cmdLine->arguments[0], "suspend") == 0)
            {
                pid_t pid = atoi(cmdLine->arguments[1]);
                int result = kill(pid, SIGTSTP);
                if (result == -1)
                {
                    perror("faild to suspend");
                }
                else
                {
                    updateProcessStatus(process_list, pid, SUSPENDED);
                }
                freeCmdLines(cmdLine);
                continue;
            }
            if (strcmp(cmdLine->arguments[0], "wake") == 0)
            {
                pid_t pid = atoi(cmdLine->arguments[1]);
                int result = kill(pid, SIGCONT);
                if (result == -1)
                {
                    perror("fail to wake");
                }
                else
                {
                    updateProcessStatus(process_list, pid, RUNNING);
                }
                freeCmdLines(cmdLine);
                continue;
            }
            if (strcmp(cmdLine->arguments[0], "kill") == 0)
            {
                pid_t pid = atoi(cmdLine->arguments[1]);
                int result = kill(pid, SIGINT);
                if (result == -1)
                {
                    perror("fail to kill");
                }
                else
                {
                    updateProcessStatus(process_list, pid, TERMINATED);
                }
                freeCmdLines(cmdLine);
                continue;
            }
            execute(cmdLine);
        }
    }
    free_history();
    freeProcessList(process_list);
    return 0;
}
