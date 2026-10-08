#ifndef SEEK_H
#define SEEK_H

#include "shell.h"

void search_tree(char* full, char* rel, char* name, int dflag, int fflag);

void seek(char* str, char* cur_dir, char* home_dir, char* prev_dir);

#endif
