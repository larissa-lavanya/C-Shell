#ifndef COMMAND_H
#define COMMAND_H

#include "shell.h"

void run_command(char* command, char* cur_dir, char* home_dir, char* prev_dir, int background, int in_child);

#endif
