#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LENGTH 80
#define MAX_ARGS 15
const char delimiter[3] = " \n";

int main() {
    // // execvp(cmd, argv); //This will run "ls -la" as if it were a command
    char command[MAX_LENGTH];
    int is_running = 1;
    int pid = 0;
    while (is_running) {
        printf("osh>"); fflush(stdout);
        fgets(command, MAX_LENGTH, stdin);
        // Tokenize args 
        char *args[MAX_ARGS] = {};
        char *token;
        int i = 0;
        token = strtok(command, delimiter);
        while (token != NULL) {
            args[i] = malloc(MAX_LENGTH + 1);
            strcpy(args[i], token);
            i += 1;
            token = strtok(NULL, delimiter);
        }
        // 
        pid = fork();
        if (pid == 0) {
            // printf("Child process\n");
            execvp(args[0], args);
        } else {
            while(wait(NULL) > 0);
        }
    }
}