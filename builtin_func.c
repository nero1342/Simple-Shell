#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>  

#include "builtin_func.h"

const int MAX_BUFFER = 256;

void mergeSort(char **lines, int low, int high);
void merge(char **line, int low, int mid, int high);

int cmd_pwd(char **args) {
    char buffer[MAX_BUFFER];
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

void mergeSort(char **lines, int low, int high) {
    for (int i = low; i <= high; ++i) {
        printf("%s\n", lines[i]);
    }
    if (low < high) {
        int mid = (low + high) / 2;
        mergeSort(lines, low, mid);
        mergeSort(lines, mid+1, high);
        merge(lines, low, mid, high);
    }
}

void merge(char **lines, int low, int mid, int high) {
    for (int i = low; i <= high; ++i) {
        printf("%s\n", lines[i]);
    }
    printf("merge %d %d %d\n", low, mid, high);
    int i = low, j = mid+1 , k = low;
    char tmp[MAX_BUFFER][MAX_BUFFER];

    while(i <= mid && j <= high) {
        if(strcmp(lines[i], lines[j]) == -1){
            strcpy(tmp[k], lines[i]);
            ++i;
        } else {
            strcpy(tmp[k], lines[j]);
            ++j;
        }
        ++k;
    }

    if(i > mid) {
        for(int h = j; h <= high; ++h) {
            strcpy(tmp[k], lines[h]);
            ++k;
        }
    } else {
        for(int h = i; h <= mid; ++h) {
            strcpy(tmp[k], lines[h]);
            ++k;
        }
    }

    // Reassign to lines
    for(int i = low; i <= high; ++i)
        strcpy(lines[i], tmp[i]);
}

int cmd_sort(char **args) {
    int i = 1, count_line = 0;
    char lines[MAX_BUFFER][MAX_BUFFER];
    char command[MAX_BUFFER];

    if (args[1] == NULL) {
        while (fgets(command, MAX_BUFFER, stdin) != NULL && count_line < MAX_BUFFER) {
            strcpy(lines[count_line], command);
            ++count_line;
        }
    } else {
        while (args[i] != NULL && count_line < MAX_BUFFER) {
            FILE *fi = fopen(args[i], "r");
            while (fgets(command, MAX_BUFFER, (FILE*) fi) != NULL && count_line < MAX_BUFFER) {
                if (command[strlen(command)-1] == '\n') 
                    command[strlen(command)-1] = '\0';
                strcpy(lines[count_line], command);
                ++count_line;
            }
            fclose(fi);
            ++i;
        }
    }

    mergeSort(lines, 0, count_line-1);

    for (int _count = 0 ; _count < count_line; ++_count) {
        printf("%s\n", lines[_count]);
    }
    
    return 0;
}

int cmd_history(char *command_history[], int num_commands) {
    for (int i = 0; i < num_commands; ++i)
        printf("%d\t%s", i + 1, command_history[i]);
    return 0;
}

int cmd_exit(char **args) {
    exit(0);
    return 0;
}
