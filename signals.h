#ifndef SIGNALS_H
#define SIGNALS_H

#include "shell.h"

void setup_signals();

void Ctrl_C(int sig);

void Ctrl_D();

void Ctrl_Z(int sig);

void shell_exit();

void ping(char* command);

#endif
