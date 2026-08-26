// Library Header Files
#include <cstdio>
#include <cstdlib>
#include <cstring>

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
        size_t length = 0;

        // Read user input command
        line = get_raw_ip();
        if(line == NULL) {
            printf("Exiting shell\n");
            break;
        }

        // if user presses enter only
        if(strlen(line)==0) {
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
                if(curr_piped_cmds == 1 && execute_builtin(&pipeline_cmds[0])) {

                }
                else {
                    bool is_it_background = pipeline_cmds[curr_piped_cmds-1].ends_with_ampersand;
                    execute_pipeline(pipeline_cmds, curr_piped_cmds, is_it_background);
                }
                
            }
            semicolon = strtok_r(NULL, ";\n", &semicolon_ptr);
        }
        free(line);
    }
    return 0;
}