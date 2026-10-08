#include "system.h"

/* Prints a message whenever a background process ends (no user action needed) */
void handle_sigchld(int sig)
{
    int i, status, saved_errno, len, done;
    pid_t result;
    char message[LEN + 128], name[LEN];

    saved_errno = errno;
    i = 0;
    while(i < process_count)
    {
        done = 0;
        result = waitpid(processes[i].PID, &status, WNOHANG | WUNTRACED | WCONTINUED);
        while(result > 0 && !done)
        {
            if(WIFSTOPPED(status))
                strcpy(processes[i].state, "Stopped");
            else if(WIFCONTINUED(status))
                strcpy(processes[i].state, "Running");
            else
            {
                int j;
                j = 0;
                while(processes[i].command[j] != ' ' && processes[i].command[j] != '\0')
                {
                    name[j] = processes[i].command[j];
                    j++;
                }
                name[j] = '\0';

                if(WIFEXITED(status) && WEXITSTATUS(status) == 0)
                    len = snprintf(message, sizeof(message), "\n%s exited normally (%d)\n", name, processes[i].PID);
                else
                    len = snprintf(message, sizeof(message), "\n%s exited abnormally (%d)\n", name, processes[i].PID);
                write(STDOUT_FILENO, message, len);

                remove_process_at(i);
                done = 1;
                break;
            }
            result = waitpid(processes[i].PID, &status, WNOHANG | WUNTRACED | WCONTINUED);
        }

        if(done)
            continue;
        if(result < 0 && errno == ECHILD)
        {
            remove_process_at(i);
            continue;
        }
        i++;
    }

    errno = saved_errno;
    return;
}

/* Children must not inherit the shell's ignored / handled signals */
void reset_child_signals()
{
    sigset_t set;

    signal(SIGINT, SIG_DFL);
    signal(SIGTSTP, SIG_DFL);
    signal(SIGTTIN, SIG_DFL);
    signal(SIGTTOU, SIG_DFL);
    signal(SIGCHLD, SIG_DFL);
    sigemptyset(&set);
    sigprocmask(SIG_SETMASK, &set, NULL);

    return;
}

/* Hands the terminal to a process group (does nothing without a terminal) */
void give_terminal(pid_t pgid)
{
    if(term_fd >= 0)
        tcsetpgrp(term_fd, pgid);
    return;
}

/* Runs a system command with the exec family, never returns */
void exec_command(char* command)
{
    char* args[256];

    split_args(command, args);
    if(args[0] == NULL)
        _exit(0);

    execvp(args[0], args);

    if(errno == ENOENT)
        printf(RED "ERROR : '%s' is not a valid command" RESET "\n", args[0]);
    else
        printf(RED "ERROR : '%s' could not be executed (%s)" RESET "\n", args[0], strerror(errno));
    fflush(stdout);
    _exit(127);
}

/* Waits for a foreground process group; a stop (Ctrl-Z) sends it to the job list */
void wait_for_foreground(pid_t pid, char* command)
{
    int status, elapsed, i;
    pid_t result;
    time_t start;

    status = 0;
    start = time(NULL);
    fg_pid = pid;

    result = waitpid(pid, &status, WUNTRACED);
    while(result < 0 && errno == EINTR)
        result = waitpid(pid, &status, WUNTRACED);

    fg_pid = -1;
    give_terminal(shell_pgid);

    if(result > 0 && WIFSTOPPED(status))
    {
        add_process(pid, command, "Stopped");
        return;
    }

    elapsed = (int)(time(NULL) - start);
    if(elapsed > 2)
    {
        i = 0;
        next_word(command, &i, note_command);
        note_time = elapsed;
    }

    return;
}

void execute_foreground_process(char* command)
{
    pid_t pid;
    char copy[LEN];

    strcpy(copy, command);
    fflush(stdout);

    pid = fork();
    if(pid == 0)
    {
        setpgid(0, 0);
        give_terminal(getpid());
        reset_child_signals();
        exec_command(copy);
    }
    else if(pid > 0)
    {
        setpgid(pid, pid);
        give_terminal(pid);
        wait_for_foreground(pid, command);
    }
    else
        printf(RED "Error" RESET "\n");

    return;
}

void execute_background_process(char* command)
{
    pid_t pid;
    char copy[LEN];
    sigset_t old;

    strcpy(copy, command);
    fflush(stdout);

    /* SIGCHLD stays blocked until the job is in the table */
    block_sigchld(&old);
    pid = fork();
    if(pid == 0)
    {
        setpgid(0, 0);
        reset_child_signals();
        exec_command(copy);
    }
    else if(pid > 0)
    {
        setpgid(pid, pid);
        add_process(pid, command, "Running");
        printf("%d\n", pid);
        fflush(stdout);
    }
    else
        printf(RED "Error" RESET "\n");
    unblock_sigchld(&old);

    return;
}
