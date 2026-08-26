#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <stdbool.h>

#define AT_MOST_COMMANDS 32
#define AT_MOST_ARGS 64

struct Command {
    char *args[AT_MOST_ARGS];
    int arg_count;
    bool ends_with_ampersand;

    char* input_file;
    char* output_file;
    bool append_mode;
};

int ip_parse(char* r_ip, struct Command commands[]);

#endif
