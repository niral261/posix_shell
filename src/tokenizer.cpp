// Library Header Files
#include <cstring>
#include <cstdlib>

// Imported Files
#include "../include/tokenizer.h"


void tokenize_command(char* cmd_str, struct Command* cmd) {
    cmd->arg_count = 0;
    cmd->ends_with_ampersand = false;
    cmd->input_file = NULL;
    cmd->output_file = NULL;
    cmd->append_mode = false;
    
    char buffer[1024];
    int buf_idx = 0;
    int i = 0;
    int len = strlen(cmd_str);
    
    while (i<len && cmd->arg_count<AT_MOST_ARGS-1) {
        while (i<len && (cmd_str[i]==' ' || cmd_str[i]=='\t' || cmd_str[i]=='\r' || cmd_str[i]=='\n')) {
            i++;
        }
        
        if (i >= len) break;
        
        buf_idx = 0;
        bool in_quotes = false;
        char quote_char = '\0';
        
    
        while (i < len) {
            char c = cmd_str[i];
            
            if (!in_quotes && (c == '"' || c == '\'')) {

                in_quotes = true;
                quote_char = c;
                i++;

            } else if (in_quotes && c == quote_char) {
                in_quotes = false;
                i++;
            } else if (!in_quotes && (c == ' ' || c == '\t' || c == '\r' || c == '\n')) {
                break;
            } else {

                buffer[buf_idx++] = c;
                i++;
            }
        }
        
        if (buf_idx > 0) {
            buffer[buf_idx] = '\0';
            
            if (strcmp(buffer, "&") == 0) {
                cmd->ends_with_ampersand = true;
            } else if (strcmp(buffer, "<") == 0) {

                while (i < len && (cmd_str[i] == ' ' || cmd_str[i] == '\t')) i++;
                int j = i;
                while (j < len && cmd_str[j] != ' ' && cmd_str[j] != '\t' && 
                       cmd_str[j] != '\n' && cmd_str[j] != '\r') j++;
                if (j > i) {
                    int flen = j - i + 1;
                    cmd->input_file = (char*)malloc(flen);
                    strncpy(cmd->input_file, &cmd_str[i], flen - 1);
                    cmd->input_file[flen - 1] = '\0';
                    i = j;
                }
            } else if (strcmp(buffer, ">") == 0) {
                while (i < len && (cmd_str[i] == ' ' || cmd_str[i] == '\t')) i++;
                int j = i;
                while (j < len && cmd_str[j] != ' ' && cmd_str[j] != '\t' && 
                    cmd_str[j] != '\n' && cmd_str[j] != '\r') j++;
                if (j > i) {
                    int flen = j - i + 1;
                    cmd->output_file = (char*)malloc(flen);
                    strncpy(cmd->output_file, &cmd_str[i], flen - 1);
                    cmd->output_file[flen - 1] = '\0';
                    cmd->append_mode = false;
                    i = j;
                }
            } else if (strcmp(buffer, ">>") == 0) {
  
                while (i < len && (cmd_str[i] == ' ' || cmd_str[i] == '\t')) i++;
                int j = i;
                while (j < len && cmd_str[j] != ' ' && cmd_str[j] != '\t' && 
                       cmd_str[j] != '\n' && cmd_str[j] != '\r') j++;
                if (j > i) {
                    int flen = j - i + 1;
                    cmd->output_file = (char*)malloc(flen);
                    strncpy(cmd->output_file, &cmd_str[i], flen - 1);
                    cmd->output_file[flen - 1] = '\0';
                    cmd->append_mode = true;
                    i = j;
                }
            } else {
                char* arg = (char*)malloc(strlen(buffer) + 1);
                strcpy(arg, buffer);
                cmd->args[cmd->arg_count] = arg;
                cmd->arg_count++;
            }
        }
    }
    
    cmd->args[cmd->arg_count] = NULL;
}

int ip_parse(char* r_ip, struct Command commands[]) {
    int total_cmd = 0;
    
    char* pipe_ptr;
    char r_ip_copy[4096];
    strcpy(r_ip_copy, r_ip);
    
    char* pipe_start = r_ip_copy;
    int cmd_idx = 0;
    
    for (int i = 0; i <= strlen(r_ip_copy) && total_cmd < AT_MOST_COMMANDS; i++) {
        bool in_quotes = false;
        char quote_char = '\0';
        

        while (i < strlen(r_ip_copy)) {
            char c = r_ip_copy[i];
            
            if (!in_quotes && (c == '"' || c == '\'')) {
                in_quotes = true;
                quote_char = c;
            } else if (in_quotes && c == quote_char) {
                in_quotes = false;
            } else if (!in_quotes && c == '|') {
                break;
            }
            i++;
        }
        

        char cmd_buffer[1024];
        int cmd_len = i - (pipe_start - r_ip_copy);
        strncpy(cmd_buffer, pipe_start, cmd_len);
        cmd_buffer[cmd_len] = '\0';
        
  
        tokenize_command(cmd_buffer, &commands[total_cmd]);
        total_cmd++;
        
        if (i < strlen(r_ip_copy) && r_ip_copy[i] == '|') {
            pipe_start = r_ip_copy + i + 1;
            i++;
        } else {
            break;
        }
    }

    return total_cmd;
}