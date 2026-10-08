#ifndef REVEAL_H
#define REVEAL_H

#include "shell.h"

int directory_exists(const char *path);

int compare_file(const void* a, const void* b);

void list(char* base, char** names, int count, int info);

void reveal(char* str, char* cur_dir, char* home_dir, char* prev_dir);

#endif
