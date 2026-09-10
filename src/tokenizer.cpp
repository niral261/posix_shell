// Library Header Files
#include <cstring>
#include <cstdlib>
#include <pwd.h>
#include <unistd.h>

// Imported Files
#include "../include/tokenizer.h"
#include <cstdio>

static char* dup_str(const char* s)
{
    size_t len = strlen(s);
    char* copy = (char*)malloc(len + 1);
    if (copy != NULL)
        strcpy(copy, s);
    return copy;
}

static char* expand_tilde_arg(const char* token)
{
    if (token[0] != '~' || (token[1] != '\0' && token[1] != '/'))
    {
        return dup_str(token);
    }

    const char* home = getenv("HOME");

    if (home == NULL)
    {
        struct passwd *pw = getpwuid(getuid());
        if (pw != NULL)
            home = pw->pw_dir;
    }

    if (home == NULL)
    {
        return dup_str(token);
    }

    const char* rest = token + 1; 
    size_t total_len = strlen(home) + strlen(rest) + 1;

    char* expanded = (char*)malloc(total_len);
    if (expanded == NULL)
        return dup_str(token);

    snprintf(expanded, total_len, "%s%s", home, rest);
    return expanded;
}

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

            char raw_name[1024];
            if (file_len >= (int)sizeof(raw_name))
                file_len = sizeof(raw_name) - 1;

            strncpy(raw_name, &cmd_str[start], file_len);
            raw_name[file_len] = '\0';

            cmd->input_file = expand_tilde_arg(raw_name);

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

            char raw_name[1024];
            if (file_len >= (int)sizeof(raw_name))
                file_len = sizeof(raw_name) - 1;

            strncpy(raw_name, &cmd_str[start], file_len);
            raw_name[file_len] = '\0';

            cmd->output_file = expand_tilde_arg(raw_name);
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
            char *arg = expand_tilde_arg(buffer);
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

            i = (int)(pipe_start - r_ip_copy) - 1;
        }
        else
        {
            break;
        }
    }

    return total_cmd;
}