// Library Header Files
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <cstdio>
#include <climits>

// Imported Files
#include "../include/builtins.h"
#include "../include/ls_command.h"
#include "../include/pinfo.h"
#include "../include/search.h"


extern char shell_directory_path[PATH_MAX];

static char last_dir[PATH_MAX] = "";

void manage_pwd() {
    char cwd[PATH_MAX];
    if(getcwd(cwd, sizeof(cwd))!=NULL) printf("%s\n", cwd);
    else perror("Error: pwd");
}

void manage_echo(struct Command *cmd) {
    for(int i=1;i< cmd->arg_count;i++) {
        printf("%s", cmd->args[i]);
        
        if(i < cmd->arg_count-1) 
            printf(" ");
    }
    printf("\n");
}

void manage_cd(struct Command *cmd) {
    if(cmd->arg_count > 2) {
        printf("INvalid arguments");
        return;
    }

    char curr_cwd[PATH_MAX];
    if(getcwd(curr_cwd, sizeof(curr_cwd))==NULL) {
        perror("cd: getcwd failed");
        return;
    }

    const char* target = NULL;

    // Part 1: 'cd-' --- prev directory
    if(strcmp(cmd->args[1], "~")==0 || cmd->arg_count==1) {
        target = shell_directory_path;
    } else if(strcmp(cmd->args[1], "-")==0) {
        if(strlen(last_dir)==0) {
            printf("cd: OLDPWD not set\n");
            return;
        }
        target = last_dir;
        printf("%s\n", last_dir);
    } else {
        target = cmd->args[1];
    }

    if(chdir(target) != 0) {
        perror("cd failed");
    } else {
        strncpy(last_dir, curr_cwd, sizeof(last_dir));
    }
}

bool execute_builtin(struct Command *cmd) {
    if(cmd->arg_count == 0 || cmd->args[0] == NULL) return false;

    if(strcmp(cmd->args[0], "cd")==0) {
        manage_cd(cmd);
        return true;
    } else if(strcmp(cmd->args[0], "pwd")==0) {
        manage_pwd();
        return true;
    } else if(strcmp(cmd->args[0], "echo")==0) {
        manage_echo(cmd);
        return true;
    } else if(strcmp(cmd->args[0], "ls")==0) {
        manage_ls(cmd);
        return true;
    } else if(strcmp(cmd->args[0], "pinfo")==0) {
        manage_pinfo(cmd);
        return true;
    } else if(strcmp(cmd->args[0], "search")==0) {
        recursive_search(cmd);
        return true;
    }
    return false;
}