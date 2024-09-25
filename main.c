#include "main.h"

#include <stdio.h>

/* fork */
#include <sys/types.h>
#include <unistd.h>

/* wait */
#include <sys/types.h>
#include <sys/wait.h>

/* execve */
#include <unistd.h>

/* strtok */
#include <string.h>

/* realloc and malloc*/
#include <stdlib.h>

int main(int argc, char * argv[]) {

    #define CHILD_PROCESS_FAILURE 12
    #define EXECVE_FAILURE 11
    #define WAIT_FAILURE 10
    #define ENVIRONMENT_FAILURE 9

    /*prototypes*/
    char ** parse_command_line(char *);
    int handle_process_execution(pid_t,int,char **);

    // ssize_t getline(char **lineptr, size_t *n, FILE *stream);

    char *buf = NULL;
    size_t count = 0;
    ssize_t nread;
    pid_t child_pid;
    int status;

    while (1) {
        /* shell prompt */
        write(STDOUT_FILENO,"LAMBIONE'S SHELL $ ",20);

        /* read user input */
        nread = getline(&buf, &count, stdin);
        if(nread == -1) {
            perror("Exiting shell");
            exit(1);
        } 
        
        /*parse command line*/
        char **command;
        command = (char **)parse_command_line(buf);

        /*get environment PATH for the executables*/
        char * path = getenv("PATH");
        if(path == NULL) {
            perror("No environment variable found");
            exit(ENVIRONMENT_FAILURE);
        }
        /*make a copy of the env paths*/
        char * path_dup = strdup(path);
        if(!path_dup) {
            exit(ENVIRONMENT_FAILURE);
        }

        /* make a copy of the first command user input*/
        char * first_cmd = strdup(command[0]);
        if(!first_cmd) {
            free(path_dup);
            exit(1);
        } 
        char * token;
        token = strtok(path_dup,":");
        while(token) {

            /* create child process */
            child_pid = fork();
            
            /* handle process execution */
            if (child_pid == -1) {
                free(path_dup);
                free(first_cmd);
                perror("Failed to create child process");
                exit(CHILD_PROCESS_FAILURE);
            }

            /* concatenate the environment path with the command */
            /* initialize command string space*/
            unsigned token_len = strlen(token);
            unsigned command_len = strlen(first_cmd);
            unsigned len = token_len + command_len;
            char * cmd;
            cmd = (char *)malloc(len + 2);
            if(!cmd) {
                free(path_dup);
                free(first_cmd);
                exit(1);
            }
            /*create the command string*/
            memcpy(cmd,token,token_len);
            cmd[token_len] = '/';
            memcpy(cmd+token_len +1,first_cmd,command_len);
            cmd[token_len + 1 + command_len] = '\0';

            /*replace in command first argument with the real executable*/
            command[0] = cmd;
            /* execute the process*/
            int st = handle_process_execution(child_pid, status, command);
            if(st != -1 && st != -1) {
                status = st;
                break;
            }

            token = strtok(NULL,":");
            free(cmd);
        }
        free(path_dup);
        free(first_cmd);
        
    }
    free(buf);
    return 0;
}


int handle_process_execution(pid_t child_pid,int status, char **command){
    int execution_trace = 0;
    if(child_pid == 0) {
        execution_trace = execve(command[0], command, NULL); 
    } else {
        /* parent waits for child otherwise*/   
        pid_t w = wait(&status);
        if(w == -1) {
            exit(WAIT_FAILURE);
        }
    }  
    /*-2 is used as return code to flag something bad happened */
    if(execution_trace == -1) return -1;
    if(status) {return status;} 
    else {return -2;}
}

char ** parse_command_line(char * buf){
    char **command;
    char *token;
    /* manage spaces and new lines*/
    token = strtok(buf," \n");
    /* initialize the command array */
    size_t size = 1024;
    command = (char **)malloc(sizeof(char *) * size);
    unsigned i = 0;
    while(token != NULL) {
        if (i >= size) {
            size *= 2;
            command = (char **)realloc(command, sizeof(char *) * size);
        }
        command[i] = token;
        token = strtok(NULL," \n");
        ++i;
    }
    // argv = {"ls", "-l", NULL} THE COMMAND ARRAY SHOULD BE NULL TERMINATED
    command[i] = NULL;
    return command;
}

/*getline function*/
// if getline fails it return -1 
    // otherwise it will return the number of characters read
    // getline dynamically allocates memory for buf that we will need to free

/*  THE FORK FUNCTION */
// Description: The fork() function simply creates a child process by duplicating the calling process.
// Return Value: On success, the PID (process ID) of the child process is returned in the parent and 0 is returned in the child. Otherwise, -1 is returned to the parent and no child is created.

        // pid_t fork(void);

/* THE WAIT FUNCTION */
// Remember that with the child execution we want the parent to wait for the complete execution of the child.
// Description: The wait() function causes the current process to wait until one of its child processes terminates. When a child process terminates, the wait() function returns the child process's exit status.
// Return Value: On success, return PID of the terminated child otherwise -1 on error.

/* THE EXECVE FUNCTION*/
// Description: The execve() function is typically used to execute a new program referred to by the pathname variable as seen above. *The pathname is an executable file which is found on your system.
    // Argv is a an array of strings
    // Envp is also an array of strings (more on that later)
    // Both Argv and Envp should be terminated by a NULL pointer.
