// Library Header Files
#include <cstring>
#include <cstdlib>

// Imported Files
#include "../include/tokenizer.h"
#include <cstdio>

bool tokenize_command(char *cmd_str, struct Command *cmd)
{
    cmd->arg_count = 0;
    cmd->ends_with_ampersand = false;
    cmd->input_file = NULL;
    cmd->output_file = NULL;
    cmd->append_mode = false;

    char buffer[1024];
    int buf_idx = 0;
    int i = 0;
    int len = strlen(cmd_str);

    while(i<len && cmd->arg_count < AT_MOST_ARGS-1)
    {
        while(i<len && (cmd_str[i] == ' ' || cmd_str[i] == '\t' || cmd_str[i] == '\r' || cmd_str[i] == '\n'))
        {
            i++;
        }

        if (i>=len)
            break;

        if (cmd_str[i] == '<') {
            i++;

            while(i < len && (cmd_str[i] == ' ' || cmd_str[i] == '\t')) {
                i++;
            }

            int start = i;

            while(i < len && cmd_str[i] != ' ' && cmd_str[i] != '\t' && cmd_str[i] != '\r' && cmd_str[i] != '\n' && cmd_str[i] != '>' && cmd_str[i] != '<' && cmd_str[i] != '|' && cmd_str[i] != '&') {
                i++;
            }

            if(i == start)
            {
                fprintf(stderr, "Syntax error: missing input file\n");
                return false;
            }

            int file_len = i - start;

            cmd->input_file = (char *)malloc(file_len + 1);

            strncpy(
                cmd->input_file,
                &cmd_str[start],
                file_len);

            cmd->input_file[file_len] = '\0';

            continue;
        }

        if (cmd_str[i] == '>')
        {
            bool append = false;
            
            i++;
            if(i < len && cmd_str[i] == '>') {
                append = true;
                i++;
            }

            while (i < len && (cmd_str[i] == ' ' || cmd_str[i] == '\t')) i++;
            
            int start = i;
            while(i<len && cmd_str[i] != ' ' && cmd_str[i] != '\t' && cmd_str[i] != '\r' && cmd_str[i] != '\n' && cmd_str[i] != '>' && cmd_str[i] != '<' && cmd_str[i] != '|' && cmd_str[i] != '&') {
                i++;
            }

            if(i == start) {
                fprintf(stderr, "Syntax error: missing output file\n");
                return false;
            }

            int file_len = i - start;
            cmd->output_file = (char *)malloc(file_len + 1);
            strncpy(cmd->output_file, &cmd_str[start], file_len);
            cmd->output_file[file_len] = '\0';
            cmd->append_mode = append;
            continue;
        }

        if (cmd_str[i] == '&') {
            i++;
            while(i<len && (cmd_str[i] == ' ' || cmd_str[i] == '\t' || cmd_str[i] == '\r' || cmd_str[i] == '\n')) {
                i++;
            }

            if(i<len) {
                fprintf(stderr, "Syntax error: & must be at the end\n");
                return false;
            }

            cmd->ends_with_ampersand = true;
            continue;
        }

        buf_idx = 0;

        bool in_quotes = false;
        char quote_char = '\0';

        while(i<len) {
            char c = cmd_str[i];

            if(!in_quotes && (c == '"' || c == '\'')) {
                in_quotes = true;
                quote_char = c;
                i++;
                continue;
            }
            if(in_quotes && c == quote_char) {
                in_quotes = false;
                i++;
                continue;
            }

            if (!in_quotes && (c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '<' || c == '>' || c == '&' || c == '|'))
                break;

            if (buf_idx < (int)sizeof(buffer) - 1)
                buffer[buf_idx++] = c;

            i++;
        }

        if(buf_idx>0) {
            buffer[buf_idx] = '\0';
            char *arg = (char *)malloc(buf_idx + 1);
            strcpy(arg, buffer);
            cmd->args[cmd->arg_count] = arg;
            cmd->arg_count++;
        }
    }
    cmd->args[cmd->arg_count] = NULL;
    return true;
}

int ip_parse(char *r_ip, struct Command commands[])
{
    int total_cmd = 0;

    char *pipe_ptr;
    char r_ip_copy[4096];
    strcpy(r_ip_copy, r_ip);

    char *pipe_start = r_ip_copy;
    int cmd_idx = 0;

    for (int i = 0; i <= strlen(r_ip_copy) && total_cmd < AT_MOST_COMMANDS; i++)
    {
        bool in_quotes = false;
        char quote_char = '\0';

        while (i < strlen(r_ip_copy))
        {
            char c = r_ip_copy[i];

            if (!in_quotes && (c == '"' || c == '\''))
            {
                in_quotes = true;
                quote_char = c;
            }
            else if (in_quotes && c == quote_char)
            {
                in_quotes = false;
            }
            else if (!in_quotes && c == '|')
            {
                break;
            }
            i++;
        }

        if(in_quotes) {
            fprintf(stderr, "Syntax error: unmatched quote\n");
            return -1;
        }

        char cmd_buffer[1024];
        int cmd_len = i - (pipe_start - r_ip_copy);

        if(cmd_len ==0) {
            fprintf(stderr, "Syntax error: invalid command\n");
            return -1;
        }
        strncpy(cmd_buffer, pipe_start, cmd_len);
        cmd_buffer[cmd_len] = '\0';

        if(!tokenize_command(cmd_buffer, &commands[total_cmd]))
            return -1;

        if(commands[total_cmd].arg_count == 0) {
            fprintf(stderr, "Syntax error: empty command\n");
            return -1;
        }
        
        
        total_cmd++;

        if (i < strlen(r_ip_copy) && r_ip_copy[i] == '|') {
            pipe_start = r_ip_copy + i + 1;
            i++;
            i++;

            while(*pipe_start == ' ' || *pipe_start == '\t')
                pipe_start++;

            if(*pipe_start == '\0') {
                fprintf(stderr, "Syntax error: invalid pipe\n");
                return -1;
            }
            if(*pipe_start == '|') {
                fprintf(stderr, "Syntax error: invalid pipe\n");
                return -1;
            }
        }
        else
        {
            break;
        }
    }

    return total_cmd;
}