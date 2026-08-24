// Library Header Files
#include <unistd.h>         // getcwd, geteuid, gethostname
#include <pwd.h>            // passwd, getpwuid
#include <cstdio>           // printf, perror
#include <cstring>          // strlen, strcpy, strcmp
#include <cstdlib>          // exit, EXIT_FAILURE
#include <sys/types.h>      // uid_t
#include <climits>          // PATH_MAX

// Imported Files
#include "../include/prompt.h"

char shell_directory_path[PATH_MAX];

void shell_init() {
    char *path_ptr = getcwd(shell_directory_path, sizeof(shell_directory_path));
    if(path_ptr == NULL) {
        perror("Error initializing shell directory path");
        exit(EXIT_FAILURE);
    }
}

void print_prompt() {
    uid_t uid = geteuid();
    struct passwd *password = getpwuid(uid);
    
    const char* username;
    if(password == NULL) {
        username = "unknown";
    } else {
        username = password->pw_name;
    }

    char sys_hostname[256];
    int isHostname = gethostname(sys_hostname, sizeof(sys_hostname));
    if(isHostname != 0) strcpy(sys_hostname, "unknown");

    char cwd[PATH_MAX];
    char *working_path_ptr = getcwd(cwd, sizeof(cwd));
    if(working_path_ptr == NULL) {
        perror("Error: Not able to get current directory");
        return;
    }


    char display_path[PATH_MAX];
    size_t curr_starting_dir_len = strlen(shell_directory_path);
    
    if(strncmp(cwd, shell_directory_path, curr_starting_dir_len) == 0) {
        display_path[0] = '~';
        strcpy(display_path+1, cwd+curr_starting_dir_len);
    } else {
        strcpy(display_path, cwd);
    }

    printf("<%s@%s:%s>", username, sys_hostname, display_path);
    fflush(stdout);    
}

