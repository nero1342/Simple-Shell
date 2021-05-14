#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "builtin_func.h"

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
    int status = chdir(args[1]);
    if (status != 0) {
        fprintf(stderr, "No such file or directory %s\n", args[1]);
        return status;
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
    exit(0);
    return 0;
}
