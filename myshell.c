#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include<sys/types.h>
#include <fcntl.h>  
// Include user header file 
#include "builtin_func.h"

#define MAX_LENGTH 80
#define MAX_ARGS 15
const char delimiter[3] = " \n";
const int num_builtins = 5;
const mode_t mode_file = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
const int MAX_HISTORY = 10;
int count = 0;
char *command_history[MAX_HISTORY];

char *builtin_command[] = {
    "apwd",
    "acd",
    "alsa",
    "asort",
    "exit"
};

int (*builtin_func[]) (char **) = {
    &cmd_pwd,
    &cmd_cd,
    &cmd_ls,
    &cmd_sort,
    &cmd_exit
};

int execute_nonbuiltin_command(char **args) {
    int run_bg = 0;
    int pid = 0;
    int i = 0;
    while (args[i] != NULL) ++i;
    if (i == 0) return 1;
    if (strcmp(args[i - 1], "&") == 0) {
        run_bg = 1;
        printf("Run in background\n");    
        args[i - 1] = '\0';
    }
    // 
    pid = fork();
    if (pid == 0) {
        if (execvp(args[0], args) == -1) {
            printf("Error when executing this command.\n");
        }
        // printf("End child process\n");
    } else {
        if (run_bg == 0)
            while(wait(NULL) > 0);
    }

    // Release the memory 
    for (int j = 0; j < i; ++j) free(args[i]);
    free(args);
    return 1;
}   

int execute_command(char **args) {
    char * input_filename = NULL, * output_filename = NULL;
    int i = 0;
    // Check whether rediect in/out or not  
    for (i = 0; args[i] != NULL ; ++i) {
        if (strcmp(args[i], "<") == 0) {
            input_filename = args[i + 1];
            args[i] = NULL;
            printf("Redirect input to %s\n", input_filename);
        }
        if (strcmp(args[i], ">") == 0) {
            output_filename = args[i + 1];
            args[i] = NULL;
            printf("Redirect output to %s\n", output_filename);
        }
    }
    if (i == 0) {
        return 0;
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
        return cmd_history(command_history, MAX_HISTORY);
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
    for (int j = 0; j < i; ++j) {
        printf("Token %d: %s\n", j, args[j]);
    }
    args[i] = NULL;
    return args;
}

void init_history() {
    for (int i = 0; i < MAX_HISTORY; ++i) 
        command_history[i] = malloc(MAX_ARGS * sizeof(char));
}

int main() {

    int saved_stdin = dup(STDIN_FILENO), saved_stdout = dup(STDOUT_FILENO);

    // // execvp(cmd, argv); //This will run "ls -la" as if it were a command
    char command[MAX_LENGTH], command_raw[MAX_LENGTH];
    char **args;

    int is_running = 1;
    int status = 0;
    init_history();
    while (is_running) {
        // Restore standard std in/out
        dup2(saved_stdin, STDIN_FILENO);
        dup2(saved_stdout, STDOUT_FILENO);
        // Read command
        printf("osh>"); fflush(stdout);
        fgets(command, MAX_LENGTH, stdin);
        strcpy(command_raw, command);
        // Tokenize args 
        args = get_params(command);
        status = execute_command(args);
        printf("Status of this command is: %d\n", status);
        // Store history of commands
        if (count == MAX_HISTORY) {
            --count;
            for (int i = 0; i < count; ++i)
                strcpy(command_history[i], command_history[i+1]);
        }
        strcpy(command_history[count], command_raw);
        ++count;
    }
}