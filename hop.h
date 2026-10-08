#ifndef HOP_H
#define HOP_H

#include "shell.h"

void resolve_path(char* arg, char* out, char* cur_dir, char* home_dir, char* prev_dir);

void dir(char* str, char* cur_dir, char* home_dir, char* prev_dir);

int hop(char* str, char* cur_dir, char* home_dir, char* prev_dir);

#endif
