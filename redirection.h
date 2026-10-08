#ifndef REDIRECTION_H
#define REDIRECTION_H

#include "shell.h"

int parse_redirect(char* stage, char* command, char* input, char* output, int* append);

void execute(char* segment, char* cur_dir, char* home_dir, char* prev_dir, int background);

void redirect(char* stage, char* cur_dir, char* home_dir, char* prev_dir, int background);

void pipes(char* segment, char** stages, int num, char* cur_dir, char* home_dir, char* prev_dir, int background);

#endif
