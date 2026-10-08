#ifndef TOKENIZE_H
#define TOKENIZE_H

#include "shell.h"

void trim(char* str);

int is_number(char* str);

int next_word(char* str, int* i, char* word);

int split_args(char* command, char* args[]);

void tokenize(char* str, char* cur_dir, char* home_dir, char* prev_dir);

#endif
