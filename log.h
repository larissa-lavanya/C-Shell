#ifndef LOG_H
#define LOG_H

#include "shell.h"

void log_print();

void log_clear();

char* log_execute(int num);

void log_command(char* str, char* cur_dir, char* home_dir, char* prev_dir);

#endif
