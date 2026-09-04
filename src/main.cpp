// Library Header Files
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>

// Imported Files
#include "../include/prompt.h"
#include "../include/tokenizer.h"
#include "../include/builtins.h"
#include "../include/process_runner.h"
#include "../include/raw_input.h"

int main() {
    shell_init();
    load_history();
    init_signal();

    while(true) {
        print_prompt();

        char *line = NULL;

        // Read user input command
        line = get_raw_ip();
        if(line == NULL) {
            printf("Exiting shell\n");
            break;
        }

        // if user presses enter only
        if(strlen(line) == 0) {
            free(line);
            continue;
        }

        add_to_history(line);

        char* semicolon_ptr;
        char* semicolon = strtok_r(line, ";\n", &semicolon_ptr);

        while(semicolon) {
            struct Command pipeline_cmds[AT_MOST_COMMANDS];
            int curr_piped_cmds = ip_parse(semicolon, pipeline_cmds);
            
            if(curr_piped_cmds > 0) {
                if(curr_piped_cmds == 1) {
                    
                    int saved_stdin = dup(0);
                    int saved_stdout = dup(1);

                    if (pipeline_cmds[0].input_file != NULL) {
                        int fd_in = open(pipeline_cmds[0].input_file, O_RDONLY);
                        if (fd_in >= 0) { 
                            dup2(fd_in, 0); 
                            close(fd_in); 
                        } else {
                            perror("Input file error");
                        }
                    }

                    if (pipeline_cmds[0].output_file != NULL) {
                        int flags = O_WRONLY | O_CREAT;
                        flags |= pipeline_cmds[0].append_mode ? O_APPEND : O_TRUNC;
                        
                        int fd_out = open(pipeline_cmds[0].output_file, flags, 0644); 
                        if (fd_out >= 0) { 
                            dup2(fd_out, 1); 
                            close(fd_out); 
                        } else {
                            perror("Output file error");
                        }
                    }

                    bool is_builtin = execute_builtin(&pipeline_cmds[0]);

                    dup2(saved_stdin, 0); dup2(saved_stdout, 1);

                    close(saved_stdin); close(saved_stdout);
                
                    if (is_builtin) {
                        semicolon = strtok_r(NULL, ";\n", &semicolon_ptr);
                        continue;
                    }
                }

                bool is_it_background = pipeline_cmds[curr_piped_cmds-1].ends_with_ampersand;
                execute_pipeline(pipeline_cmds, curr_piped_cmds, is_it_background);
            }
            
            semicolon = strtok_r(NULL, ";\n", &semicolon_ptr);
        }
        free(line);
    }
    return 0;
}