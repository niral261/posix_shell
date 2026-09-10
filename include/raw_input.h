#ifndef RAW_INPUT_H
#define RAW_INPUT_H

#define MAX_HISTORY 20
#define MAX_INPUT_LEN 1024

extern char cmd_hist[MAX_HISTORY][MAX_INPUT_LEN];
extern int hist_cnt;

char* get_raw_ip();

void load_history();
void save_history();
void add_to_history(char* cmd);

#endif