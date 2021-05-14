#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>

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
    char * dir = NULL;
    int op_a = 0, op_l = 0;
    for (int i = 1; args[i] != NULL; ++i) {
        if (args[i][0] == '-') {
            for (int j = 1; j < strlen(args[i]); ++j) {
                if (args[i][j] == 'a') op_a = 1;
                if (args[i][j] == 'l') op_l = 1;
            }
            continue;
        } 
        if (dir == NULL) {
            dir = args[i];
        }
    }
    if (dir == NULL) {
        dir = "./";
    }
    printf("Dir %s, %d %d\n", dir, op_a, op_l);
    struct dirent *d;
    struct stat this_stat;
    DIR * mydir = opendir(dir);
    if (!mydir) {
        return 1;
    }
    int buf[512];
    while ((d = readdir(mydir)) != NULL) {
        if (!op_a && d->d_name[0] == '.') {
            continue;
        }
        if (op_l) {
            sprintf(buf, "%s/%s", dir, d->d_name);
            stat(buf, &this_stat);
            // Stat here
            printf("%10d", this_stat.st_size);

            printf(" %s\n", d->d_name);
        } else {
            printf("%s ", d->d_name);
        }
    }
    printf("\n");
    return 0;
}

int cmd_sort(char **args) {
    return 0;
}

int cmd_history(char *command_history[], int num_commands) {
    for (int i = 0; i < num_commands; ++i)
        printf("%d: %s", i + 1, command_history[i]);
    return 0;
}

int cmd_exit(char **args) {
    exit(0);
    return 0;
}
