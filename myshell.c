#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LENGTH 80
#define MAX_ARGS 15
const char delimiter[3] = " \n";
const int num_builtins = 6;

int cmd_pwd(char **args);
int cmd_cd(char **args);
int cmd_ls(char **args);
int cmd_sort(char **args);
int cmd_history(char **args);
int cmd_exit(char **args);

char *builtin_command[] = {
    "pwd",
    "cd",
    "lsa",
    "sort",
    "history",
    "exit"
};

int (*builtin_func[]) (char **) = {
    &cmd_pwd,
    &cmd_cd,
    &cmd_ls,
    &cmd_sort,
    &cmd_history,
    &cmd_exit
};

int cmd_pwd(char **args) {
    char buffer[256];
    if (getcwd(buffer, sizeof(buffer)) == NULL) {
        perror("can't get current dir");
        return 1;
    }
    printf("Current directory is: %s\n", buffer);
    return 0;
}

int cmd_cd(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "Expected argument(destination directory) for this argument\n");
        return 1;
    }
    if (chdir(args[1]) != 0) {
        fprintf(stderr, "No such file or directory %s\n", args[1]);
        return 1;
    } 
    return 0;
}

int cmd_ls(char **args) {
    return 0;
}

int cmd_sort(char **args) {
    return 0;
}

int cmd_history(char **args) {
    return 0;
}

int cmd_exit(char **args) {
    return 0;
}



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
    for (int i = 0; i < num_builtins; ++i) {
        if (strcmp(args[0], builtin_command[i]) == 0) {
            return (*builtin_func[i])(args);
        }
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
    args[i] = NULL;
    return args;
}

int main() {
    // // execvp(cmd, argv); //This will run "ls -la" as if it were a command
    char command[MAX_LENGTH];
    char **args;

    int is_running = 1;
    int status = 0;

    while (is_running) {
        // Read command
        printf("osh>"); fflush(stdout);
        fgets(command, MAX_LENGTH, stdin);
        // Tokenize args 
        args = get_params(command);
        status = execute_command(args);
        printf("Status of this command is: %d\n", status);
    }
}