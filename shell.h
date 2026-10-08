#ifndef SHELL_H
#define SHELL_H

#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <termios.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdbool.h>
#include <ctype.h>
#include <sys/select.h>

#define LEN 4096
#define MAX_PROCESSES 256
#define MAX_STAGES 64

#define WHITE "\033[0;37m"
#define BLUE "\033[1;34m"
#define GREEN "\033[1;32m"
#define RED "\033[1;31m"
#define RESET "\033[0m"

typedef struct
{
    int PID;
    char command[LEN];
    char state[10];
    int fg;
} Process;

/* Global shell state (shared with the signal handlers) */
extern Process processes[MAX_PROCESSES];
extern int process_count;
extern int fg_pid;
extern int term_fd;
extern pid_t shell_pgid;
extern char log_path[LEN];
extern char note_command[LEN];
extern int note_time;

#include "tokenize.h"
#include "redirection.h"
#include "command.h"
#include "myshrc.h"
#include "hop.h"
#include "reveal.h"
#include "seek.h"
#include "log_store.h"
#include "log.h"
#include "proclore.h"
#include "activities.h"
#include "system.h"
#include "signals.h"
#include "foreground.h"
#include "background.h"
#include "iman.h"
#include "neonate.h"
#include "display.h"

#endif
