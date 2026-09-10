#ifndef BUILTINS_H
#define BUILTINS_H

#include "tokenizer.h"

bool execute_builtin(struct Command *cmd);

void manage_cd(struct Command *cmd);
void manage_pwd();
void manage_echo(struct Command *cmd);

#endif