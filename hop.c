#include "hop.h"

/* Turns ~, ~/x, - and -/x into real paths (".", ".." and relative ones are left to the OS) */
void resolve_path(char* arg, char* out, char* cur_dir, char* home_dir, char* prev_dir)
{
    if(arg[0] == '~' && (arg[1] == '/' || arg[1] == '\0'))
    {
        strcpy(out, home_dir);
        strncat(out, &arg[1], LEN - strlen(out) - 1);
    }
    else if(arg[0] == '-' && (arg[1] == '/' || arg[1] == '\0'))
    {
        strcpy(out, prev_dir);
        strncat(out, &arg[1], LEN - strlen(out) - 1);
    }
    else
    {
        strncpy(out, arg, LEN - 1);
        out[LEN - 1] = '\0';
    }

    return;
}

void dir(char* str, char* cur_dir, char* home_dir, char* prev_dir)
{
    char target[LEN], old_dir[LEN];

    resolve_path(str, target, cur_dir, home_dir, prev_dir);
    strcpy(old_dir, cur_dir);

    /* The shell really changes its working directory */
    if(chdir(target) != 0)
    {
        printf(RED "Invalid path" RESET "\n");
        return;
    }

    strcpy(prev_dir, old_dir);
    if(getcwd(cur_dir, LEN) == NULL)
    {
        printf(RED "Error" RESET "\n");
        return;
    }
    printf(BLUE "%s" RESET "\n", cur_dir);

    return;
}

int hop(char* str, char* cur_dir, char* home_dir, char* prev_dir)
{
    int i, count;
    char word[LEN];

    i = 0;
    count = 0;
    while(next_word(str, &i, word))
    {
        dir(word, cur_dir, home_dir, prev_dir);
        count++;
    }

    /* No argument means home */
    if(count == 0)
        dir("~", cur_dir, home_dir, prev_dir);

    return 0;
}
