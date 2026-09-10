// Library Header Files
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>

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

bool is_directory(const char* path) {
    struct stat stats;
    if (stat(path, &stats) == -1) return false;
    return S_ISDIR(stats.st_mode);
}

void search_nested_directory(const char* dir_path, const char* file_prefix, 
                             char* matched_entities[], int* match_count, int max_matches) {
    DIR *folder_stream = opendir(dir_path);
    if (!folder_stream) return;

    struct dirent *file_entry;
    while ((file_entry = readdir(folder_stream)) != NULL) {
        if (file_prefix[0] != '.' && file_entry->d_name[0] == '.') 
            continue;

        if (strncmp(file_entry->d_name, file_prefix, strlen(file_prefix)) == 0) {
            bool is_duplicate = false;
            
            for (int m = 0; m < *match_count; m++) {
                if (strcmp(matched_entities[m], file_entry->d_name) == 0) {
                    is_duplicate = true; 
                    break;
                }
            }
            
            if (!is_duplicate && *match_count < max_matches) {
                matched_entities[*match_count] = strdup(file_entry->d_name);
                (*match_count)++;
            }
        }
    }
    closedir(folder_stream);
}

void tab_completion(char *line_spooler, int *cursor_pos) {
    int typing_word_idx = *cursor_pos;
    while(typing_word_idx > 0 && line_spooler[typing_word_idx - 1] != ' ' && line_spooler[typing_word_idx - 1] != '\t') {
        typing_word_idx--;
    }

    bool is_first_word = (typing_word_idx == 0) ? true : false;

    char typed_pref[512];
    int pref_len = *cursor_pos - typing_word_idx;
    
    if (pref_len >= 511) return;

    strncpy(typed_pref, &line_spooler[typing_word_idx], pref_len);
    typed_pref[pref_len] = '\0';

    int match = 0;
    char* matched_entities[1024];

    char search_base_dir[512] = ".";
    char file_prefix[256] = "";
    int last_slash = -1;

    for (int i = pref_len - 1; i >= 0; i--) {
        if (typed_pref[i] == '/') {
            last_slash = i;
            break;
        }
    }

    if (last_slash != -1) {
        strncpy(search_base_dir, typed_pref, last_slash);
        search_base_dir[last_slash] = '\0';
        strcpy(file_prefix, &typed_pref[last_slash + 1]);
    } else {
        if (is_first_word) {
            strcpy(file_prefix, typed_pref);
            const char* search_dirs[3] = {"/usr/bin", "/bin", "."};
            for (int d = 0; d < 3; d++) {
                search_nested_directory(search_dirs[d], typed_pref, matched_entities, &match, 1024);
            }
        } else {
            strcpy(file_prefix, typed_pref);
            search_nested_directory(".", file_prefix, matched_entities, &match, 1024);
        }
        
        if (match > 0 || is_first_word) {
            goto handle_matches;
        }
    }

    if (last_slash != -1) {
        search_nested_directory(search_base_dir, file_prefix, matched_entities, &match, 1024);
    }

handle_matches:
    if (match == 1) {
        int remaining_len = strlen(matched_entities[0]) - strlen(file_prefix);
        strcpy(&line_spooler[*cursor_pos], &matched_entities[0][strlen(file_prefix)]);
        
        write(STDOUT_FILENO, &matched_entities[0][strlen(file_prefix)], remaining_len);
        *cursor_pos += remaining_len;

        char full_path[1024];

        if (last_slash != -1) {
            snprintf(full_path, sizeof(full_path), "%s/%s", search_base_dir, matched_entities[0]);
        } else if (is_first_word) {
            snprintf(full_path, sizeof(full_path), "/usr/bin/%s", matched_entities[0]);
            if (!is_directory(full_path)) {
                snprintf(full_path, sizeof(full_path), "/bin/%s", matched_entities[0]);
            }
            if (!is_directory(full_path)) {
                snprintf(full_path, sizeof(full_path), "./%s", matched_entities[0]);
            }
        } else {
            snprintf(full_path, sizeof(full_path), "./%s", matched_entities[0]);
        }

        if (is_directory(full_path)) {
            line_spooler[*cursor_pos] = '/';
            write(STDOUT_FILENO, "/", 1);
            (*cursor_pos)++;
        } else {
            line_spooler[*cursor_pos] = ' ';
            write(STDOUT_FILENO, " ", 1);
            (*cursor_pos)++;
        }
        
        line_spooler[*cursor_pos] = '\0';

    } else if (match > 1) {
        char shared_pref[256];
        strcpy(shared_pref, matched_entities[0]);
        
        for (int i = 1; i < match; i++) {
            common_prefix(shared_pref, matched_entities[i]);
        }

        int shared_len = strlen(shared_pref);
        int file_pref_len = strlen(file_prefix);
        
        if (shared_len > file_pref_len) {
            int added_len = shared_len - file_pref_len;
            
            strcpy(&line_spooler[*cursor_pos], &shared_pref[file_pref_len]);
            write(STDOUT_FILENO, &shared_pref[file_pref_len], added_len);
            
            *cursor_pos += added_len;
            line_spooler[*cursor_pos] = '\0';
        } else {
            write(STDOUT_FILENO, "\n", 1);
            
            for (int i = 0; i < match; i++) {
                char full_path[512];
                if (last_slash != -1) {
                    snprintf(full_path, sizeof(full_path), "%s/%s", search_base_dir, matched_entities[i]);
                } else {
                    snprintf(full_path, sizeof(full_path), "./%s", matched_entities[i]);
                }
                
                write(STDOUT_FILENO, matched_entities[i], strlen(matched_entities[i]));
                if (is_directory(full_path)) {
                    write(STDOUT_FILENO, "/", 1);
                }
                write(STDOUT_FILENO, "  ", 2);
            }
            
            write(STDOUT_FILENO, "\n", 1);
            print_prompt();

            fflush(stdout);

            write(STDOUT_FILENO, line_spooler, *cursor_pos);
        }
    }

    for (int i = 0; i < match; i++) {
        free(matched_entities[i]);
    }
}