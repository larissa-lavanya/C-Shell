#include "log_store.h"

/* The log lives in a file so it is shared by every session */
int read_log(char lines[][LEN])
{
    FILE* fptr;
    int i;

    fptr = fopen(log_path, "r");
    if(fptr == NULL)
        return 0;

    i = 0;
    while(i < 15 && fgets(lines[i], LEN, fptr) != NULL)
    {
        lines[i][strcspn(lines[i], "\n")] = '\0';
        i++;
    }
    fclose(fptr);

    return i;
}

void write_log(char lines[][LEN], int num)
{
    FILE* fptr;
    int i;

    fptr = fopen(log_path, "w");
    if(fptr == NULL)
    {
        printf(RED "Error" RESET "\n");
        return;
    }

    for(i=0 ; i<num ; i++)
        fprintf(fptr, "%s\n", lines[i]);
    fclose(fptr);

    return;
}

/* 1 if any ';' / '&' / '|' separated part of the string is a log command */
int has_log_command(char* str)
{
    int i, j;
    char part[LEN], word[LEN];

    i = 0;
    j = 0;
    while(1)
    {
        if(str[i] == ';' || str[i] == '&' || str[i] == '|' || str[i] == '\0')
        {
            int k;
            part[j] = '\0';
            k = 0;
            if(next_word(part, &k, word) && strcmp(word, "log") == 0)
                return 1;
            j = 0;
            if(str[i] == '\0')
                break;
        }
        else
            part[j++] = str[i];
        i++;
    }

    return 0;
}

void log_store(char* str)
{
    char lines[15][LEN];
    int num, i;

    /* A line containing a log command is never stored */
    if(has_log_command(str))
        return;

    num = read_log(lines);

    /* Same as the previous command, nothing to store */
    if(num > 0 && strcmp(lines[num-1], str) == 0)
        return;

    /* Oldest command is overwritten once 15 are stored */
    if(num == 15)
    {
        for(i=1 ; i<15 ; i++)
            strcpy(lines[i-1], lines[i]);
        num--;
    }
    strncpy(lines[num], str, LEN - 1);
    lines[num][LEN - 1] = '\0';
    num++;

    write_log(lines, num);

    return;
}
