#include "neonate.h"

/* The last number in /proc/loadavg is the most recently created process id */
pid_t get_pid()
{
    FILE* fptr;
    double a, b, c;
    int running, total, last;

    fptr = fopen("/proc/loadavg", "r");
    if(fptr == NULL)
        return -1;

    if(fscanf(fptr, "%lf %lf %lf %d/%d %d", &a, &b, &c, &running, &total, &last) != 6)
        last = -1;
    fclose(fptr);

    return last;
}

void neonate(char* command)
{
    int i, interval, tty;
    char word[LEN], ch;
    struct termios oldt, newt;
    fd_set fds;
    struct timeval tv;
    time_t next, now;
    pid_t recent_pid;

    i = 0;
    if(!next_word(command, &i, word) || strcmp(word, "-n") != 0 || !next_word(command, &i, word) || !is_number(word))
    {
        printf(RED "Invalid command" RESET "\n");
        return;
    }
    interval = atoi(word);
    if(interval <= 0)
    {
        printf(RED "Invalid command" RESET "\n");
        return;
    }

    /* Key presses are read without waiting for enter */
    tty = (tcgetattr(STDIN_FILENO, &oldt) == 0);
    if(tty)
    {
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        newt.c_cc[VMIN] = 1;
        newt.c_cc[VTIME] = 0;
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    }

    next = time(NULL);
    while(1)
    {
        now = time(NULL);
        if(now >= next)
        {
            recent_pid = get_pid();
            if(recent_pid < 0)
            {
                printf(RED "Error" RESET "\n");
                break;
            }
            printf("%d\n", recent_pid);
            fflush(stdout);
            next = now + interval;
            continue;
        }

        /* Sleep until the next print or until a key arrives */
        FD_ZERO(&fds);
        FD_SET(STDIN_FILENO, &fds);
        tv.tv_sec = next - now;
        tv.tv_usec = 0;
        if(select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv) > 0)
        {
            if(read(STDIN_FILENO, &ch, 1) <= 0 || ch == 'x')
                break;
        }
    }

    if(tty)
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

    return;
}
