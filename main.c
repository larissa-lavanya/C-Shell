#include "display.h"

/* Global shell state */
Process processes[MAX_PROCESSES];
int process_count = 0;
int fg_pid = -1;
int term_fd = -1;
pid_t shell_pgid;
char log_path[LEN];
char note_command[LEN];
int note_time = 0;

int main(int argc, char *argv[])
{
    note_command[0] = '\0';

    setup_signals();

    /* Job control only makes sense when stdin is a real terminal */
    if(isatty(STDIN_FILENO))
    {
        term_fd = fcntl(STDIN_FILENO, F_DUPFD_CLOEXEC, 10);
        setpgid(0, 0);
        shell_pgid = getpgrp();
        if(term_fd >= 0)
            tcsetpgrp(term_fd, shell_pgid);
    }
    else
        shell_pgid = getpgrp();

    display();

    return 0;
}
