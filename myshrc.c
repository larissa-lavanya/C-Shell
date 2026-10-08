#include "myshrc.h"

typedef struct
{
    char name[64];
    char value[LEN];
} Alias;

typedef struct
{
    char name[64];
    char body[32][LEN];
    int lines;
} Function;

static Alias aliases[64];
static int alias_count = 0;
static Function functions[32];
static int function_count = 0;
static int function_depth = 0;

/* Removes a trailing '#' or '//' comment (outside quotes) */
static void strip_comment(char* line)
{
    int i;
    char quote;

    quote = '\0';
    for(i=0 ; line[i] != '\0' ; i++)
    {
        if(quote == '\0' && (line[i] == '"' || line[i] == '\''))
            quote = line[i];
        else if(quote != '\0' && line[i] == quote)
            quote = '\0';
        else if(quote == '\0' && (i == 0 || line[i-1] == ' ') && (line[i] == '#' || (line[i] == '/' && line[i+1] == '/')))
        {
            line[i] = '\0';
            break;
        }
    }

    return;
}

/* Reads aliases and functions from .myshrc in the shell's home directory */
void load_myshrc(char* home_dir)
{
    char path[LEN], line[LEN];
    FILE* fptr;
    int in_function, i;

    snprintf(path, sizeof(path), "%s/.myshrc", home_dir);
    fptr = fopen(path, "r");
    if(fptr == NULL)
        return;

    in_function = 0;
    while(fgets(line, sizeof(line), fptr) != NULL)
    {
        for(i=0 ; line[i] != '\0' ; i++)
        {
            if(line[i] == '\t' || line[i] == '\n' || line[i] == '\r')
                line[i] = ' ';
        }
        strip_comment(line);
        trim(line);
        if(line[0] == '\0')
            continue;

        if(in_function)
        {
            Function* f = &functions[function_count - 1];
            if(strcmp(line, "{") == 0)
                continue;
            if(strcmp(line, "}") == 0)
            {
                in_function = 0;
                continue;
            }
            if(f->lines < 32)
            {
                strcpy(f->body[f->lines], line);
                f->lines++;
            }
            continue;
        }

        char* paren = strstr(line, "()");
        if(paren != NULL)
        {
            /* name() { ... }  or  func name() { ... } */
            char name[LEN];
            *paren = '\0';
            strcpy(name, line);
            trim(name);
            if(strncmp(name, "func ", 5) == 0)
            {
                memmove(name, name + 5, strlen(name + 5) + 1);
                trim(name);
            }
            if(function_count < 32 && name[0] != '\0')
            {
                strncpy(functions[function_count].name, name, 63);
                functions[function_count].name[63] = '\0';
                functions[function_count].lines = 0;
                function_count++;
                in_function = 1;
            }
            continue;
        }

        /* name = command  or  alias name = command */
        char* text = line;
        if(strncmp(text, "alias ", 6) == 0)
            text = text + 6;
        char* equal = strchr(text, '=');
        if(equal != NULL && alias_count < 64)
        {
            char value[LEN];
            *equal = '\0';
            strcpy(value, equal + 1);
            trim(text);
            trim(value);
            int len = strlen(value);
            if(len >= 2 && (value[0] == '"' || value[0] == '\'') && value[len-1] == value[0])
            {
                value[len-1] = '\0';
                memmove(value, value + 1, strlen(value));
            }
            if(text[0] != '\0' && value[0] != '\0')
            {
                strncpy(aliases[alias_count].name, text, 63);
                aliases[alias_count].name[63] = '\0';
                strcpy(aliases[alias_count].value, value);
                alias_count++;
            }
        }
    }
    fclose(fptr);

    return;
}

/* Replaces the first word by its alias (only single word aliases are supported) */
int expand_alias(char* command)
{
    int i, j, depth, changed;
    char first[LEN], temp[LEN];

    changed = 0;
    depth = 0;
    while(depth < 5)
    {
        i = 0;
        while(command[i] != ' ' && command[i] != '\0')
        {
            first[i] = command[i];
            i++;
        }
        first[i] = '\0';

        for(j=0 ; j<alias_count ; j++)
        {
            if(strcmp(aliases[j].name, first) == 0)
                break;
        }
        if(j == alias_count)
            break;

        snprintf(temp, sizeof(temp), "%s%s", aliases[j].value, command + i);
        strcpy(command, temp);
        changed = 1;

        /* alias ls = ls -la must not loop */
        i = 0;
        while(command[i] != ' ' && command[i] != '\0')
            i++;
        if(strncmp(command, first, i) == 0 && (int)strlen(first) == i)
            break;
        depth++;
    }

    return changed;
}

/* Runs a user defined function, $1 $2 ... are replaced by its arguments */
int run_function(char* command, char* cur_dir, char* home_dir, char* prev_dir)
{
    int i, j, k, l, argc;
    char name[LEN], word[LEN], line[LEN];
    char (*args)[LEN];

    i = 0;
    next_word(command, &i, name);
    for(j=0 ; j<function_count ; j++)
    {
        if(strcmp(functions[j].name, name) == 0)
            break;
    }
    if(j == function_count)
        return 0;

    if(function_depth >= 5)
    {
        printf(RED "Function nested too deeply" RESET "\n");
        return 1;
    }

    args = malloc(9 * sizeof(*args));
    argc = 0;
    while(argc < 9 && next_word(command, &i, word))
    {
        strcpy(args[argc], word);
        argc++;
    }

    function_depth++;
    for(k=0 ; k<functions[j].lines ; k++)
    {
        char* body = functions[j].body[k];
        l = 0;
        for(i=0 ; body[i] != '\0' && l < LEN - 1 ; i++)
        {
            if(body[i] == '$' && body[i+1] >= '1' && body[i+1] <= '9')
            {
                int index = body[i+1] - '1';
                if(index < argc)
                {
                    int m;
                    for(m=0 ; args[index][m] != '\0' && l < LEN - 1 ; m++)
                        line[l++] = args[index][m];
                }
                i++;
            }
            else
                line[l++] = body[i];
        }
        line[l] = '\0';
        tokenize(line, cur_dir, home_dir, prev_dir);
    }
    function_depth--;

    free(args);
    return 1;
}
