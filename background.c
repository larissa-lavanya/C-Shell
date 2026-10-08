#include "background.h"

/* bg <pid> : a stopped job continues running in the background */
void b2b(char* command)
{
    int i, pid, index;
    char word[LEN];
    sigset_t old;

    i = 0;
    if(!next_word(command, &i, word) || !is_number(word))
    {
        printf(RED "No such process found" RESET "\n");
        return;
    }
    pid = atoi(word);

    block_sigchld(&old);
    index = find_process(pid);
    if(index < 0)
    {
        unblock_sigchld(&old);
        printf(RED "No such process found" RESET "\n");
        return;
    }

    if(strcmp(processes[index].state, "Stopped") == 0)
    {
        kill(-pid, SIGCONT);
        strcpy(processes[index].state, "Running");
    }
    unblock_sigchld(&old);

    return;
}
