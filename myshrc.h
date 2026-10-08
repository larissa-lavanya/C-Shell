#ifndef MYSHRC_H
#define MYSHRC_H

#include "shell.h"

void load_myshrc(char* home_dir);

int expand_alias(char* command);

int run_function(char* command, char* cur_dir, char* home_dir, char* prev_dir);

#endif
