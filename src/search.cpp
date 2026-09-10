// Library Header Files
#include <dirent.h>
#include <cstdio>
#include <sys/stat.h>
#include <cstring>
#include <linux/limits.h>

// Imported Files
#include "../include/search.h"
#include "../include/tokenizer.h"
#include "../include/ls_command.h"

bool wonder_dir_tree(char* node_path, char* target) {
    DIR* curr_dir_stream = opendir(node_path);

    if(curr_dir_stream == NULL) {
        return false;
    }

    struct dirent* child_entry;

    while(child_entry=readdir(curr_dir_stream)) {
        if(strcmp(child_entry->d_name, ".")==0 || strcmp(child_entry->d_name, "..")==0) {
            continue;
        }
        if(strcmp(child_entry->d_name, target)==0) {
            closedir(curr_dir_stream);
            return true;
        }

        char full_path[PATH_MAX];
        
        snprintf(full_path, sizeof(full_path), "%s/%s", node_path, child_entry->d_name);
        struct stat entity_meta;
        if(stat(full_path, &entity_meta)==0) {
            if(S_ISDIR(entity_meta.st_mode)) {
                bool caught_in_child = wonder_dir_tree(full_path, target);

                if(caught_in_child) {
                    closedir(curr_dir_stream);
                    return true;
                }
            }
        }
    }
    closedir(curr_dir_stream);
    return false;
}


void recursive_search(struct Command *cmd) {
    if(cmd->arg_count < 2) {
        printf("Error: Missing search target\n");
        return;
    }

    char *sought_item = cmd->args[1];
    if(wonder_dir_tree(".", sought_item)) {
        printf("True\n");
    } else {
        printf("False\n");
    }
}