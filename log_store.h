#ifndef LOG_STORE_H
#define LOG_STORE_H

#include "shell.h"

int read_log(char lines[][LEN]);

void write_log(char lines[][LEN], int num);

int has_log_command(char* str);

void log_store(char* str);

#endif
