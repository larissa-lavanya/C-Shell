#include "signals.h"

void setup_signals()
{
    struct sigaction sa;

    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    sa.sa_handler = handle_sigchld;
    sigaction(SIGCHLD, &sa, NULL);

    sa.sa_handler = Ctrl_C;
    sigaction(SIGINT, &sa, NULL);

    sa.sa_handler = Ctrl_Z;
    sigaction(SIGTSTP, &sa, NULL);

    /* The shell must be able to hand the terminal around */
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);

    return;
}

/* The terminal only signals the foreground group, so the shell itself ignores these */
void Ctrl_C(int sig)
{
    if(fg_pid > 0)
        kill(-fg_pid, SIGINT);
    return;
}

void Ctrl_Z(int sig)
{
    if(fg_pid > 0)
        kill(-fg_pid, SIGTSTP);
    return;
}

/* Kills every job and leaves the shell */
void shell_exit()
{
    int i;
    for(i=0 ; i<process_count ; i++)
    {
        kill(-processes[i].PID, SIGKILL);
        kill(processes[i].PID, SIGKILL);
    }
    exit(0);
}

void Ctrl_D()
{
    shell_exit();
    return;
}

void ping(char* command)
{
    int i, pid, signal_number;
    char word[LEN];

    i = 0;
    if(!next_word(command, &i, word) || !is_number(word))
    {
        printf(RED "Invalid arguments" RESET "\n");
        return;
    }
    pid = atoi(word);

    if(!next_word(command, &i, word) || !is_number(word))
    {
        printf(RED "Invalid arguments" RESET "\n");
        return;
    }
    signal_number = atoi(word);

    /* Signal number is taken modulo 32 */
    signal_number = signal_number % 32;
    if(kill(pid, signal_number) == -1)
    {
        if(errno == ESRCH)
            printf(RED "No such process found" RESET "\n");
        else
            perror("Error sending signal");
    }
    else
        printf("Sent signal %d to process with pid %d\n", signal_number, pid);

    return;
}
