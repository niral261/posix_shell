// Library Header Files
#include <cstdio>
#include <dirent.h>
#include <sys/types.h>
#include <cstring>

// Imported Files
#include "../include/ls_command.h"

void list_dir(const char* path, bool is_hidden_visible) {
    DIR *dir = opendir(path);

    if(dir == NULL) {
        perror("ls error");
        return;
    }

    struct dirent *entry;
    while((entry = readdir(dir)) != NULL) {
        if(!is_hidden_visible && entry->d_name[0] == '.') 
            continue;

        printf("%s\n", entry->d_name);
    }
    closedir(dir);
}

void manage_ls(struct Command *cmd) {
    bool is_hidden_visible = false;
    bool list_format = false;

    char* target[AT_MOST_ARGS];
    int target_count = 0;

    for(int i=1;i<cmd->arg_count;i++) {
        if(cmd->args[i][0] == '-') {
            int j=1;
            while(cmd->args[i][j] != '\0') {
                if(cmd->args[i][j] == '1') {
                    list_format = true;
                } else if(cmd->args[i][j] == 'a') {
                    is_hidden_visible = true;
                }
                j++;
            }
        } else {
            target[target_count] = cmd->args[i];
            target_count++;
        }
    }

    if(target_count==0)
        list_dir(".", is_hidden_visible);
    else {
        for(int i=0;i<target_count;i++) {
            if(target_count > 1) 
                printf("%s:\n", target[i]);

            list_dir(target[i], is_hidden_visible);
            
            if(i<target_count-1)
                printf("\n");
        }
    }
}