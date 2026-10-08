#include "log.h"

void log_print()
{
    char lines[15][LEN];
    int num, i;

    num = read_log(lines);
    for(i=0 ; i<num ; i++)
        printf("%s\n", lines[i]);

    return;
}

void log_clear()
{
    FILE* fptr;

    fptr = fopen(log_path, "w");
    if(fptr == NULL)
    {
        printf(RED "Error" RESET "\n");
        return;
    }
    fclose(fptr);

    return;
}

/* num = 1 is the most recent command */
char* log_execute(int num)
{
    static char line[LEN];
    char lines[15][LEN];
    int count;

    count = read_log(lines);
    if(num < 1 || num > count)
    {
        printf(RED "Invalid index" RESET "\n");
        return NULL;
    }

    strcpy(line, lines[count - num]);
    return line;
}

void log_command(char* str, char* cur_dir, char* home_dir, char* prev_dir)
{
    int i;
    char word[LEN];

    i = 0;
    if(!next_word(str, &i, word))
    {
        log_print();
        return;
    }

    if(strcmp(word, "purge") == 0)
        log_clear();
    else if(strcmp(word, "execute") == 0)
    {
        char* line;
        if(!next_word(str, &i, word) || !is_number(word))
        {
            printf(RED "Invalid command" RESET "\n");
            return;
        }

        line = log_execute(atoi(word));
        if(line != NULL)
            tokenize(line, cur_dir, home_dir, prev_dir);
    }
    else
        printf(RED "Invalid command" RESET "\n");

    return;
}
