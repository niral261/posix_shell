// Library Header Files
#include <cstdio>
#include <cstdlib>
#include <cstring>

// Imported Files
#include "../include/prompt.h"
#include "../include/tokenizer.h"
#include "../include/builtins.h"
#include "../include/process_runner.h"

int main() {
    shell_init();
    init_signal();

    struct Command commands[AT_MOST_COMMANDS];

    while(true) {
        print_prompt();

        char *line = NULL;
        size_t length = 0;

        // Read user input command
        if(getline(&line, &length, stdin) == -1) {
            printf("Exiting shell\n");
            free(line);
            break;
        }

        // if user presses enter only
        if(strcmp(line, "\n")==0) {
            free(line);
            continue;
        }

        int curr_cmd_count = ip_parse(line, commands);

        for(int i=0;i<curr_cmd_count;i++) {
            if(commands[i].arg_count == 0) continue;
            if(!execute_builtin(&commands[i])) {
                dispatch_external_cmd(&commands[i]);
            }

        }
        free(line);
    }
    return 0;
}