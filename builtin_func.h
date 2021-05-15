#ifndef BUILTIN_FUNC_H
#define BUILTIN_FUNC_H 

int cmd_pwd(char **args);
int cmd_cd(char **args);
int cmd_ls(char **args);
int cmd_sort(char **args);
int cmd_history(char *command_history[], int num_commands);
int cmd_exit(char **args);

#endif // BUILTIN_FUNC_H 
