#ifndef ACTIVITIES_H
#define ACTIVITIES_H

#include "shell.h"

void block_sigchld(sigset_t* old);

void unblock_sigchld(sigset_t* old);

void add_process(int pid, char* command, char* state);

int find_process(int pid);

void remove_process_at(int index);

int take_process(int pid, char* command);

int compare_process(const void* a, const void* b);

void print_activities();

#endif
