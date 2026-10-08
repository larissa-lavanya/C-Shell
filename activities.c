#include "activities.h"

void block_sigchld(sigset_t* old)
{
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGCHLD);
    sigprocmask(SIG_BLOCK, &set, old);
    return;
}

void unblock_sigchld(sigset_t* old)
{
    sigprocmask(SIG_SETMASK, old, NULL);
    return;
}

void add_process(int pid, char* command, char* state)
{
    sigset_t old;
    block_sigchld(&old);

    if(process_count < MAX_PROCESSES)
    {
        processes[process_count].PID = pid;
        strncpy(processes[process_count].command, command, LEN - 1);
        processes[process_count].command[LEN - 1] = '\0';
        strcpy(processes[process_count].state, state);
        processes[process_count].fg = 0;
        process_count++;
    }
    else
        printf(RED "Too many processes to keep track of" RESET "\n");

    unblock_sigchld(&old);
    return;
}

/* Callers must have SIGCHLD blocked */
int find_process(int pid)
{
    int i;
    for(i=0 ; i<process_count ; i++)
    {
        if(processes[i].PID == pid)
            return i;
    }
    return -1;
}

/* Callers must have SIGCHLD blocked (or be the SIGCHLD handler) */
void remove_process_at(int index)
{
    int i;
    for(i=index ; i<process_count-1 ; i++)
        processes[i] = processes[i+1];
    process_count--;
    return;
}

/* Removes a process from the table and hands back its command (used by fg) */
int take_process(int pid, char* command)
{
    sigset_t old;
    int index;
    block_sigchld(&old);

    index = find_process(pid);
    if(index >= 0)
    {
        strcpy(command, processes[index].command);
        remove_process_at(index);
    }

    unblock_sigchld(&old);
    return (index >= 0);
}

int compare_process(const void* a, const void* b)
{
    Process* A = (Process*)a;
    Process* B = (Process*)b;
    int temp;
    temp = strcmp(A->command, B->command);
    if(temp == 0)
        return A->PID - B->PID;
    return temp;
}

void print_activities()
{
    sigset_t old;
    Process copy[MAX_PROCESSES];
    int count, i;

    block_sigchld(&old);
    count = process_count;
    for(i=0 ; i<count ; i++)
        copy[i] = processes[i];
    unblock_sigchld(&old);

    qsort(copy, count, sizeof(Process), compare_process);

    for(i=0 ; i<count ; i++)
        printf("%d : %s - %s\n", copy[i].PID, copy[i].command, copy[i].state);

    return;
}
