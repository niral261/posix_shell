// Library Header Files
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <cstring>
#include <linux/limits.h>

// Imported Files
#include "../include/pinfo.h"
#include "../include/ls_command.h"

extern char shell_directory_path[PATH_MAX];

void switch_to_correct_path(char *runable_path) {
    size_t root_len = strlen(shell_directory_path);
    bool is_inside_shell_directory = strncmp(runable_path, shell_directory_path, root_len) == 0 &&
        (runable_path[root_len] == '\0' || runable_path[root_len] == '/');
    if(is_inside_shell_directory) {
        printf("Runnable Path: ~%s\n", runable_path + root_len);
    } else {
        printf("Runnable Path: ~%s\n", runable_path);
    }
}

void manage_pinfo(struct Command *cmd) {
    pid_t pid;

    if(cmd->arg_count > 2) {
        printf("Invalid arguments\n");
        return;
    }

    if(cmd->arg_count == 1) {
        pid = getpid();
    } else {
        char *e_ptr;
        long value = strtol(cmd->args[1], &e_ptr, 10);
        if(*e_ptr!='\0' || value <= 0) {
            printf("Invalid PID\n");
            return;
        }
        pid = (pid_t)value;
    }

    printf("pid: %d\n", pid);

    char file_path[PATH_MAX];
    snprintf(file_path, sizeof(file_path), "/proc/%d/stat", pid);

    FILE *meta_file = fopen(file_path, "r");
    if(!meta_file) {
        perror("Error: Either process does not exist or permission denied");
        return;
    }

    char spooler[1<<10];
    if(fgets(spooler, sizeof(spooler), meta_file) == NULL) {
        perror("Error: Can't read the file\n");
        fclose(meta_file);
        return;
    }
    fclose(meta_file);

    char *end_with_comma = strrchr(spooler, ')');
    if(end_with_comma == NULL) {
        printf("Error: Parsing a meta file\n");
        return;
    }

    char *meta_data = end_with_comma + 2;
    char stat = '?';

    int pgr = 0, tpgid = 0;
    unsigned long long vsize = 0;
    
    char* token = strtok(meta_data, " ");
    int index = 3;

    while(token) {
        if(index == 3) stat = token[0];
        else if(index == 5) pgr = atoi(token);
        else if(index == 8) tpgid = atoi(token);
        else if(index == 23) { vsize = strtoul(token, NULL, 10); break; }
        token = strtok(NULL, " ");
        index++;
    }

    bool is_it_foreground = (pgr != tpgid ? false : true);
    printf("Process status: %c%s\n", stat, is_it_foreground ? "+" : "");
        printf("Memory: %llu {Virtual Memory}\n", vsize);

    char runnable_path[PATH_MAX];
    char runnable_target[PATH_MAX];

    snprintf(runnable_path, sizeof(runnable_path), "/proc/%d/exe", pid);

    ssize_t len = readlink(runnable_path, runnable_target, sizeof(runnable_target)-1);
    if(len!=-1) {
        runnable_target[len] = '\0';
        switch_to_correct_path(runnable_target);
    } else {
        printf("Runnable Path: [Zombie Process or permission is denied]\n");
    }
}