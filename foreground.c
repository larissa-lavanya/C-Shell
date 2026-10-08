#include "foreground.h"

/* fg <pid> : resume a job, give it the terminal and wait for it */
void b2f(char* command)
{
    int i, pid;
    char word[LEN], name[LEN];

    i = 0;
    if(!next_word(command, &i, word) || !is_number(word))
    {
        printf(RED "No such process found" RESET "\n");
        return;
    }
    pid = atoi(word);

    if(!take_process(pid, name))
    {
        printf(RED "No such process found" RESET "\n");
        return;
    }

    give_terminal(pid);
    kill(-pid, SIGCONT);
    wait_for_foreground(pid, name);

    return;
}
