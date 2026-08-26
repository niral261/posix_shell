// Library Header Files
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <dirent.h>

// Imported Files
#include "../include/prompt.h"
#include "../include/autocomplete.h"

void common_prefix(char* common_str, const char* new_str) {
    int idx = 0;
    while(common_str[idx] != '\0' && new_str[idx] != '\0' && common_str[idx] == new_str[idx]) {
        idx++;
    }
    common_str[idx] = '\0'; 
}

void tab_completion(char *line_spooler, int *cursor_pos) {
    int typing_word_idx = *cursor_pos;
    while(typing_word_idx > 0 && line_spooler[typing_word_idx - 1]!=' ') typing_word_idx--;

    bool is_first_word = (typing_word_idx == 0) ? true : false;

    char typed_pref[256];
    int pref_len = *cursor_pos - typing_word_idx;
    
    strncpy(typed_pref, &line_spooler[typing_word_idx], pref_len);
    typed_pref[pref_len] = '\0';

    int match = 0, dir_cnt = 0;
    
    char* search_dir[3];
    char* matched_entities[1024];

    if(is_first_word) {
        search_dir[0] = "/usr/bin"; 
        search_dir[1] = "/bin";
        search_dir[2] = ".";
        dir_cnt = 3;
    } else {
        search_dir[0] = ".";
        dir_cnt = 1;
    }

    for(int d = 0; d < dir_cnt; d++) {
        DIR *folder_stream = opendir(search_dir[d]);
        if (!folder_stream) continue;

        struct dirent *file_entry;
        while ((file_entry = readdir(folder_stream)) != NULL) {
            if (typed_pref[0] != '.' && file_entry->d_name[0] == '.') 
                continue;

            if (strncmp(file_entry->d_name, typed_pref, pref_len) == 0) {
                bool is_duplicate = false;
                
                for (int m = 0; m < match; m++) {
                    if (strcmp(matched_entities[m], file_entry->d_name) == 0) {
                        is_duplicate = true; break;
                    }
                }
                
                if (!is_duplicate && match < 1024) {
                    matched_entities[match] = strdup(file_entry->d_name);
                    match++;
                }
            }
        }
        closedir(folder_stream);
    }

    if (match == 1) {
        int remaining_len = strlen(matched_entities[0]) - pref_len;
        strcpy(&line_spooler[*cursor_pos], &matched_entities[0][pref_len]);
        
        write(STDOUT_FILENO, &matched_entities[0][pref_len], remaining_len);
        *cursor_pos += remaining_len;

        line_spooler[*cursor_pos] = ' ';
        write(STDOUT_FILENO, " ", 1);
        
        (*cursor_pos)++;
        line_spooler[*cursor_pos] = '\0';

    } else if (match > 1) {
        char shared_pref[256];
        strcpy(shared_pref, matched_entities[0]);
        
        for (int i = 1; i < match; i++) {
            common_prefix(shared_pref, matched_entities[i]);
        }

        int shared_len = strlen(shared_pref);
        if (shared_len > pref_len) {
            int added_len = shared_len - pref_len;
            
            strcpy(&line_spooler[*cursor_pos], &shared_pref[pref_len]);
            write(STDOUT_FILENO, &shared_pref[pref_len], added_len);
            
            *cursor_pos += added_len;
            line_spooler[*cursor_pos] = '\0';
        } 
        
        else {
            write(STDOUT_FILENO, "\n", 1);
            
            for (int i = 0; i < match; i++) {
                write(STDOUT_FILENO, matched_entities[i], strlen(matched_entities[i]));
                write(STDOUT_FILENO, "  ", 2);
            }
            
            write(STDOUT_FILENO, "\n", 1);
            print_prompt();
            write(STDOUT_FILENO, line_spooler, *cursor_pos);
        }
    }

    for (int i = 0; i < match; i++) {
        free(matched_entities[i]);
    }
}