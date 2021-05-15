#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include<sys/types.h>
#include <fcntl.h>  

// Include user header file 
#include "myshell.h"

const char shell_name[] = "osh>";
const char delimiter[3] = " \n";
const int num_builtins = 5;
const mode_t mode_file = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
const int MAX_HISTORY = 10;
const int MAX_LENGTH = 80;
const int MAX_ARGS = 15;

void handler_sigint(int sig) {
    fprintf(stderr, "\nCtrl-C detected!!\n%s", shell_name);
    fflush(stdout);
}
void handler_sigtstp(int sig) {
    fprintf(stderr, "\nCtrl-Z detected!!\n%s", shell_name);
    fflush(stdout);
}

int child_exit = 0;
int run_in_bg = 0;
void handler_sigchld(int sig) {
    // something here
    pid_t pid = 0;
    fprintf(stderr, "SIGCHLD detected!!\n");
    if (run_in_bg == 0) {
        while ((pid = waitpid(-1, NULL, WNOHANG)) == 0);
        fprintf(stderr, "Process with pid %d finished.\n", pid);
        child_exit = 1;
    }

}

const char *builtin_command[] = {
    "pwd",
    "cd",
    "ls",
    "sort",
    "exit",
    "!!"
};

int count = 0;

char *command_history[MAX_HISTORY];

int (*builtin_func[]) (char **) = {
    &cmd_pwd,
    &cmd_cd,
    &cmd_ls,
    &cmd_sort,
    &cmd_exit
};

char **get_params(char *command);
int execute_command(char **args);
int execute_nonbuiltin_command(char **args);
int execute_pipe(char **argv1, char **argv2);

void init_history();
void save_command(char * command);

void init_history() {
    count = 0;
    for (int i = 0; i < MAX_HISTORY; ++i) 
        command_history[i] = malloc(MAX_ARGS * sizeof(char));
}

void save_command(char * command) {
    if (count == MAX_HISTORY) {
        --count;
        for (int i = 0; i < count; ++i)
            strcpy(command_history[i], command_history[i+1]);
    }
    strcpy(command_history[count], command);
    ++count;
}

int get_last_command(char * command) {
    if (count == 0) {
        return -1;
    }
    strcpy(command, command_history[count - 1]);
    return 0;
}

int main() {
    signal(SIGINT, handler_sigint);
    signal(SIGTSTP, handler_sigtstp);
    signal(SIGCHLD, handler_sigchld);

    // Save stdin/out and restore after each command 
    int saved_stdin = dup(STDIN_FILENO), saved_stdout = dup(STDOUT_FILENO);

    char command[MAX_LENGTH], command_raw[MAX_LENGTH];
    char **args;
    int is_running = 1;
    int status = 0;
    init_history();
    
    while (is_running) {
        // Read command
        printf("osh>"); fflush(stdout);
        fgets(command, MAX_LENGTH, stdin); 
        strcpy(command_raw, command);
        printf("Command: %s\n", command);

        // Tokenize args and execute the command
        args = get_params(command);
        if (args[0] != NULL && strcmp(args[0], "!!") == 0) {
            if (get_last_command(command) != 0) {
                fprintf(stderr, "No commands in history.\n");
                continue;
            }
            printf("Last command: %s", command);
            strcpy(command_raw, command);
            args = get_params(command);
        }
        status = execute_command(args);

        // Restore standard std in/out
        dup2(saved_stdin, STDIN_FILENO);
        fflush(stdin);
        // close(saved_stdin);
        dup2(saved_stdout, STDOUT_FILENO);
        fflush(stdout);
        // close(saved_stdout);
        printf("Status of this command is: %d\n", status);

        // Store history of commands
        save_command(command_raw);
    }
}

int execute_nonbuiltin_command(char **args) {
    int pid = 0;
    pid = fork();
    if (pid == 0) {
        if (execvp(args[0], args) == -1) {
            printf("Error when executing this command.\n");
            exit(1);
        }
        exit(0);
    } else {
        if (run_in_bg == 0) {
            // printf("Pid: %d\n", pid);
            while(child_exit == 0) {
                // printf("Waiting....\n");
            }
            child_exit = 0;
        }
    }

    // Release the memory 
    free(args);
    return 0;
}   

int execute_command(char **args) {
    run_in_bg = 0;
    char * input_filename = NULL, * output_filename = NULL;
    int i = 0;
    // Check whether rediect in/out or not  
    for (i = 0; args[i] != NULL ; ++i) {
        if (strcmp(args[i], "<") == 0) {
            input_filename = args[i + 1];
            args[i] = NULL;
            if (strcmp(args[0], "sort") != 0)
                printf("Redirect input to %s\n", input_filename);

        } else if (strcmp(args[i], ">") == 0) {
            output_filename = args[i + 1];
            args[i] = NULL;
            printf("Redirect output to %s\n", output_filename);
            continue;
        } else if (strcmp(args[i], "|") == 0) {
            char ** argv2 = &args[i + 1];
            args[i] = NULL;
            return execute_pipe(args, argv2);
        }
    }
    if (i == 0) return 0;
    // Check background
    if (strcmp(args[i - 1], "&") == 0) {
        run_in_bg = 1;
        printf("Run in background\n");    
        args[i - 1] = '\0';
    }
    // Redirect in/out
    int fi, fo;
    if (input_filename != NULL) {
        fi = open(input_filename, O_RDONLY, mode_file);
        if (fi == -1) {
            fprintf(stderr, "Error when opening file to read\n");
            return 1;
        }
        dup2(fi, STDIN_FILENO); 
    } 
    if (output_filename != NULL) {
        fo = open(output_filename, O_WRONLY | O_CREAT, mode_file);
        if (fo == -1) {
            fprintf(stderr, "Error when opening file to write\n");
            return 1;
        }
        dup2(fo, STDOUT_FILENO); 
    }
    // Check whether builtin function or not
    for (int i = 0; i < num_builtins; ++i) {
        if (strcmp(args[0], builtin_command[i]) == 0) {
            return (*builtin_func[i])(args);
        }
    }
    if (strcmp(args[0], "history") == 0) {
        return cmd_history(command_history, count);
    }
    return execute_nonbuiltin_command(args);
}

char **get_params(char *command) {
    char **args = malloc(MAX_ARGS * sizeof(char*));
    char *token;
    int i = 0;
    token = strtok(command, delimiter);
    while (token != NULL) {
        args[i] = malloc(MAX_LENGTH + 1);
        strcpy(args[i], token);
        i += 1;
        token = strtok(NULL, delimiter);
    }
    // for (int j = 0; j < i; ++j) {
    //     printf("Token %d: %s\n", j, args[j]);
    // }
    args[i] = NULL;
    return args;
}

int execute_pipe(char **argv1, char **argv2) {
    printf("Pipe detected!!\n");
    // for(int j = 0; argv1[j] != NULL; ++j) {
    //     printf("1_%d: %s\n", j, argv1[j]);
    // } 
    // for(int j = 0; argv2[j] != NULL; ++j) {
    //     printf("2_%d: %s\n", j, argv2[j]);
    // } 
    int fds[2];
    pipe(fds);
    int i = 0;
    
    while (i < 2) {
        pid_t pid = fork();
        if (pid == -1) {
            fprintf(stderr, "Error when fork!!\n");
            return 1;
        }
        if (pid == 0) {
        // child process
            if (i == 0) {
                close(fds[0]);
                dup2(fds[1], 1);
                if (execvp(argv1[0], argv1) == 1) {
                    fprintf(stderr, "Error when using execvp!!\n");
                    exit(1);
                } 
                exit(0);
            } else if (i == 1) {
                close(fds[1]);
                dup2(fds[0], 0);
                if (execvp(argv2[0], argv2) == -1) {
                    fprintf(stderr, "Error when using execvp!!\n");
                    exit(1);
                }
                exit(0);
            }
        } else {
            ++i;
        }
    } 
    free(argv1);
    return 0;
}