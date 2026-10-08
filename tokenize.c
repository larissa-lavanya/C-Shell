#include "tokenize.h"

void trim(char* str)
{
    int i, j;
    i = 0;
    while(str[i] == ' ' || str[i] == '\t' || str[i] == '\n' || str[i] == '\r')
        i++;

    j = 0;
    while(str[i] != '\0')
        str[j++] = str[i++];
    str[j] = '\0';

    j--;
    while(j >= 0 && (str[j] == ' ' || str[j] == '\t' || str[j] == '\n' || str[j] == '\r'))
        j--;
    str[j+1] = '\0';

    return;
}

int is_number(char* str)
{
    int i;
    i = 0;
    if(str[0] == '\0')
        return 0;
    while(str[i] != '\0')
    {
        if(str[i] < '0' || str[i] > '9')
            return 0;
        i++;
    }
    return 1;
}

/* Copies the next word (quotes removed) into word, returns 0 if none is left */
int next_word(char* str, int* i, char* word)
{
    int j;
    j = 0;
    while(str[*i] == ' ' || str[*i] == '\t')
        (*i)++;
    if(str[*i] == '\0')
        return 0;

    while(str[*i] != ' ' && str[*i] != '\t' && str[*i] != '\0')
    {
        if(str[*i] == '"' || str[*i] == '\'')
        {
            char quote;
            quote = str[*i];
            (*i)++;
            while(str[*i] != quote && str[*i] != '\0')
                word[j++] = str[(*i)++];
            if(str[*i] != '\0')
                (*i)++;
        }
        else
            word[j++] = str[(*i)++];
    }
    word[j] = '\0';

    return 1;
}

/* Splits a command into args in place (quotes removed), used for exec */
int split_args(char* command, char* args[])
{
    int count, more;
    char *r, *w;
    count = 0;
    r = command;
    w = command;

    while(*r != '\0')
    {
        while(*r == ' ' || *r == '\t')
            r++;
        if(*r == '\0' || count >= 255)
            break;

        args[count++] = w;
        while(*r != '\0' && *r != ' ' && *r != '\t')
        {
            if(*r == '"' || *r == '\'')
            {
                char quote;
                quote = *r;
                r++;
                while(*r != quote && *r != '\0')
                    *w++ = *r++;
                if(*r != '\0')
                    r++;
            }
            else
                *w++ = *r++;
        }
        more = (*r != '\0');
        *w++ = '\0';
        if(more)
            r++;
    }
    args[count] = NULL;

    return count;
}

/* Splits the line on ';' and '&' and remembers which of them ended each command */
void tokenize(char* str, char* cur_dir, char* home_dir, char* prev_dir)
{
    char line[LEN], segment[LEN];
    int i, j, background;
    char quote, ch;

    strncpy(line, str, LEN - 1);
    line[LEN - 1] = '\0';

    i = 0;
    j = 0;
    quote = '\0';
    while(1)
    {
        ch = line[i];
        if(ch == '\t')
            ch = ' ';

        if(quote == '\0' && (ch == '"' || ch == '\''))
            quote = ch;
        else if(quote != '\0' && ch == quote)
            quote = '\0';

        if(ch == '\0' || (quote == '\0' && (ch == ';' || ch == '&')))
        {
            background = (ch == '&');
            segment[j] = '\0';
            trim(segment);
            if(segment[0] != '\0')
                execute(segment, cur_dir, home_dir, prev_dir, background);
            j = 0;
            if(ch == '\0')
                break;
        }
        else
            segment[j++] = ch;
        i++;
    }

    return;
}
