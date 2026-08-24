// Library Header Files
#include <cstring>
#include <cstdlib>

// Imported Files
#include "../include/tokenizer.h"

int ip_parse(char* r_ip, struct Command commands[]) {
    int total_cmd = 0;

    char* pipe_ptr;
    char* pipe = strtok_r(r_ip, "|", &pipe_ptr);

    char* r_commands[AT_MOST_COMMANDS];
    char* semicolon_token = strtok(r_ip, ";\r\n");

    while(semicolon_token!=NULL && total_cmd < AT_MOST_COMMANDS) {
        r_commands[total_cmd] = semicolon_token;
        total_cmd++;
        semicolon_token = strtok(NULL, ";\r\n");
    }

    for(int i=0;i<total_cmd;i++) {
        commands[i].ends_with_ampersand = false;
        commands[i].arg_count = 0;

        char* arg_token = strtok(r_commands[i], " \t\r\n");

        while(arg_token!=NULL && commands[i].arg_count < AT_MOST_ARGS-1) {
            if(strcmp(arg_token, "&") != 0) {
                commands[i].args[commands[i].arg_count] = arg_token;
                commands[i].arg_count++;
            } else {
                commands[i].ends_with_ampersand = true;
            }
            arg_token = strtok(NULL, " \t\r\n");
        }
        commands[i].args[commands[i].arg_count] = NULL;

    }
    return total_cmd;
}