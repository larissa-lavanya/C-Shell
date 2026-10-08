#ifndef SYSTEM_H
#define SYSTEM_H

#include "shell.h"

void handle_sigchld(int sig);

void reset_child_signals();

void give_terminal(pid_t pgid);

void exec_command(char* command);

void wait_for_foreground(pid_t pid, char* command);

void execute_foreground_process(char* command);

void execute_background_process(char* command);

#endif
