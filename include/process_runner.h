#ifndef PROCESS_RUNNER_H
#define PROCESS_RUNNER_H

#include "tokenizer.h"
#include <sys/types.h>

extern pid_t g_running_fg_pid;

void dispatch_external_cmd(struct Command *cmd);
void init_signal();

#endif