// Library Header Files
#include <cstring>
#include <cstdlib>

// Imported Files
#include "../include/tokenizer.h"

int ip_parse(char* r_ip, struct Command commands[]) {
    int total_cmd = 0;

    char* pipe_ptr;
    char* pipe = strtok_r(r_ip, "|", &pipe_ptr);

    while(pipe!=NULL && total_cmd < AT_MOST_COMMANDS) {
        commands[total_cmd].arg_count = 0;
        commands[total_cmd].ends_with_ampersand = false;

        commands[total_cmd].input_file = NULL;
        commands[total_cmd].output_file = NULL;
        commands[total_cmd].append_mode = false;

        char* space_ptr;

        char* arg_token = strtok_r(pipe, " \t\r\n", &space_ptr);
        
        while(arg_token!=NULL && commands[total_cmd].arg_count + 1 < AT_MOST_ARGS) {
            if(strcmp(arg_token, "&") == 0) commands[total_cmd].ends_with_ampersand = true;
            else if(strcmp(arg_token, "<") == 0) {
                arg_token = strtok_r(NULL, " \t\r\n", &space_ptr);
                commands[total_cmd].input_file = arg_token;
            }
            else if(strcmp(arg_token, ">") == 0) {
                arg_token = strtok_r(NULL, " \t\r\n", &space_ptr);
                commands[total_cmd].output_file = arg_token;
                commands[total_cmd].append_mode = false;
            }
            else if(strcmp(arg_token, ">>") == 0) {
                arg_token = strtok_r(NULL, " \t\r\n", &space_ptr);
                commands[total_cmd].output_file = arg_token;
                commands[total_cmd].append_mode = true;
            }
            else {
                commands[total_cmd].args[commands[total_cmd].arg_count] = arg_token;
                commands[total_cmd].arg_count++;
            }

            if(arg_token != NULL) arg_token = strtok_r(NULL, " \t\r\n", &space_ptr);
        }

        commands[total_cmd].args[commands[total_cmd].arg_count] = NULL;
        total_cmd++;

        pipe = strtok_r(NULL, "|", &pipe_ptr);
    }

    return total_cmd;
}