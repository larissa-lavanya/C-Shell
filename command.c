#include "command.h"

/* Single place where every command (user defined or system) is dispatched */
void run_command(char* command, char* cur_dir, char* home_dir, char* prev_dir, int background, int in_child)
{
    char line[LEN], name[LEN];
    char* rest;
    int i;

    strncpy(line, command, LEN - 1);
    line[LEN - 1] = '\0';
    trim(line);
    if(line[0] == '\0')
        return;

    expand_alias(line);

    i = 0;
    next_word(line, &i, name);
    rest = &line[i];

    if(run_function(line, cur_dir, home_dir, prev_dir))
        return;

    if(strcmp(name, "hop") == 0)
        hop(rest, cur_dir, home_dir, prev_dir);
    else if(strcmp(name, "reveal") == 0)
        reveal(rest, cur_dir, home_dir, prev_dir);
    else if(strcmp(name, "log") == 0)
        log_command(rest, cur_dir, home_dir, prev_dir);
    else if(strcmp(name, "proclore") == 0)
        proclore(rest, home_dir);
    else if(strcmp(name, "seek") == 0)
        seek(rest, cur_dir, home_dir, prev_dir);
    else if(strcmp(name, "activities") == 0)
        print_activities();
    else if(strcmp(name, "ping") == 0)
        ping(rest);
    else if(strcmp(name, "fg") == 0)
        b2f(rest);
    else if(strcmp(name, "bg") == 0)
        b2b(rest);
    else if(strcmp(name, "iMan") == 0)
        iman(rest);
    else if(strcmp(name, "neonate") == 0)
        neonate(rest);
    else if(strcmp(name, "exit") == 0 || strcmp(name, "EXIT") == 0)
    {
        if(in_child)
            _exit(0);
        shell_exit();
    }
    else
    {
        /* Anything else is a system command (exec family) */
        if(in_child)
            exec_command(line);
        else if(background)
            execute_background_process(line);
        else
            execute_foreground_process(line);
    }

    return;
}
