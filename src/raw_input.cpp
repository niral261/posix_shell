// Library Header Files
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
<<<<<<< HEAD
#include <cerrno>
#include <csignal>
=======
>>>>>>> 1d0817cf6ce6de577538e8101b688a0c2d074130
#include <termios.h>
#include <unistd.h>
#include <linux/limits.h>
#include <fcntl.h>

// Imported Files
#include "../include/raw_input.h"
#include "../include/autocomplete.h"

extern char shell_directory_path[PATH_MAX];

<<<<<<< HEAD
extern volatile sig_atomic_t g_line_interrupted;

=======
>>>>>>> 1d0817cf6ce6de577538e8101b688a0c2d074130
int hist_cnt = 0;
char cmd_hist[MAX_HISTORY][MAX_INPUT_LEN];

void start_raw_mode(struct termios *original) {
    tcgetattr(0, original);
    struct termios raw = *original;
    raw.c_lflag = raw.c_lflag & ~(ICANON | ECHO);
    tcsetattr(0, TCSAFLUSH, &raw);
}

void end_raw_mode(struct termios *original) {
    tcsetattr(0, TCSAFLUSH, original);
}

char *get_raw_ip() {
    struct termios original;
    start_raw_mode(&original);

    char* spooler = (char*)malloc(MAX_INPUT_LEN);
    memset(spooler, 0, MAX_INPUT_LEN);
    int pos = 0, hist_idx = hist_cnt;
    char c;
<<<<<<< HEAD

    while (true) {
        ssize_t n = read(0, &c, 1);

        if (n < 0) {
            if (errno == EINTR) {
                if (g_line_interrupted) {
                    g_line_interrupted = 0;
                    memset(spooler, 0, MAX_INPUT_LEN);
                    pos = 0;
                    break;
                }
                continue;
            }
            break;
        }

        if (n == 0) {
            break;
        }

=======
    while(read(0,&c,1)==1) {
>>>>>>> 1d0817cf6ce6de577538e8101b688a0c2d074130
        if(c=='\n') {
            write(1, "\n", 1);
            break;
        }

        else if(c==4) {
            free(spooler);
            end_raw_mode(&original);
            return NULL;
        }

        else if((c==8 || c==127 ) && (pos > 0)) {
            spooler[--pos] = '\0';
            write(1,"\b \b", 3);
        }

        else if(c=='\t') {
            tab_completion(spooler, &pos);
        }

        else if(c=='\033') {
            char seq[3];

<<<<<<< HEAD
            if((read(0, &seq[0], 1)<=0) || (read(0, &seq[1], 1)<=0))
=======
            if((read(0, &seq[0], 1)==0) || (read(0, &seq[1], 1)==0))
>>>>>>> 1d0817cf6ce6de577538e8101b688a0c2d074130
                continue;

            if(seq[0]=='[') {
                if(seq[1]=='A') {
                    if(hist_idx > 0) {
                        hist_idx--;

                        while(pos > 0) {
                            write(1, "\b \b", 3);
                            pos--;
                        }

                        strcpy(spooler, cmd_hist[hist_idx]);
                        pos = strlen(spooler);
                        write(1, spooler, pos);
                    }
                } 
                else if(seq[1] == 'B') {
                    if(hist_idx +1 < hist_cnt) {
                        hist_idx++;
                        
                        while(pos > 0) {
                            write(1, "\b \b", 3);
                            pos--;
                        }

                        strcpy(spooler, cmd_hist[hist_idx]);
                        pos = strlen(spooler);
                        write(1, spooler, pos);
                    } 
                    else if(hist_idx +1 == hist_cnt) {
                        hist_idx++;

                        while(pos > 0) {
                            write(1, "\b \b", 3);
                            pos--;
                        }

                        memset(spooler, 0, MAX_INPUT_LEN);
                    }
                }
            } 
        }

        else if(isprint(c)) {
            spooler[pos++] = c;
            write(1,&c,1);
        }

    }

    end_raw_mode(&original);
    return spooler;

}

void load_history() {
    char hist_file_path[PATH_MAX];
    snprintf(hist_file_path, sizeof(hist_file_path), "%s/.shell_history", shell_directory_path);

    FILE *archieve = fopen(hist_file_path, "r");

    if(!archieve) return;

    char logged_line[MAX_INPUT_LEN];

    hist_cnt = 0;
    while(fgets(logged_line, sizeof(logged_line), archieve) && hist_cnt < MAX_HISTORY) {
        logged_line[strcspn(logged_line, "\n")] = '\0';
        strcpy(cmd_hist[hist_cnt], logged_line);
        hist_cnt++;
    }
    fclose(archieve);
}

void save_history() {
    char hist_file_path[PATH_MAX];
    snprintf(hist_file_path, sizeof(hist_file_path), "%s/.shell_history", shell_directory_path);

    FILE *archieve = fopen(hist_file_path, "w");
    
    if(!archieve) return;

    for(int i=0;i<hist_cnt;i++) {
        fprintf(archieve, "%s\n", cmd_hist[i]);
    }
    fclose(archieve);
}

void add_to_history(char* new_cmd) {
    if(strlen(new_cmd)==0) return;
    if(hist_cnt>0 && strcmp(cmd_hist[hist_cnt-1], new_cmd) == 0) return;
    if(hist_cnt==MAX_HISTORY) {
        int shift_idx=1;
        while(shift_idx<MAX_HISTORY) {
            strcpy(cmd_hist[shift_idx-1], cmd_hist[shift_idx]);
            shift_idx++;
        }
        hist_cnt = MAX_HISTORY - 1;
    }

    strcpy(cmd_hist[hist_cnt], new_cmd);
    hist_cnt++;
    save_history();
<<<<<<< HEAD
}
=======
}
>>>>>>> 1d0817cf6ce6de577538e8101b688a0c2d074130
