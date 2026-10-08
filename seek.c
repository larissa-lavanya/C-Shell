#include "seek.h"

typedef struct
{
    char path[LEN];
    int is_dir;
} Match;

static Match* matches = NULL;
static int match_count = 0;
static int match_cap = 0;

static int compare_names(const void* a, const void* b)
{
    return strcmp(*((char* const*)a), *((char* const*)b));
}

static void add_match(char* path, int is_dir)
{
    if(match_count == match_cap)
    {
        if(match_cap == 0)
            match_cap = 16;
        else
            match_cap = match_cap * 2;
        matches = (Match*)realloc(matches, match_cap * sizeof(Match));
    }
    strncpy(matches[match_count].path, path, LEN - 1);
    matches[match_count].path[LEN - 1] = '\0';
    matches[match_count].is_dir = is_dir;
    match_count++;

    return;
}

/* Walks the whole tree below full, recording matches as paths relative to the target */
void search_tree(char* full, char* rel, char* name, int dflag, int fflag)
{
    DIR* dr;
    struct dirent* ent;
    char** names;
    int count, cap, i, is_dir;
    char child_full[LEN], child_rel[LEN];
    struct stat st;

    dr = opendir(full);
    if(dr == NULL)
        return;

    names = NULL;
    count = 0;
    cap = 0;
    ent = readdir(dr);
    while(ent != NULL)
    {
        if(strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0)
        {
            if(count == cap)
            {
                if(cap == 0)
                    cap = 16;
                else
                    cap = cap * 2;
                names = (char**)realloc(names, cap * sizeof(char*));
            }
            names[count] = strdup(ent->d_name);
            count++;
        }
        ent = readdir(dr);
    }
    closedir(dr);

    qsort(names, count, sizeof(char*), compare_names);

    for(i=0 ; i<count ; i++)
    {
        snprintf(child_full, sizeof(child_full), "%s/%s", full, names[i]);
        snprintf(child_rel, sizeof(child_rel), "%s/%s", rel, names[i]);
        is_dir = (lstat(child_full, &st) == 0 && S_ISDIR(st.st_mode));

        /* Exact name or the search text as a prefix */
        if(strncmp(names[i], name, strlen(name)) == 0)
        {
            if(!(dflag && !is_dir) && !(fflag && is_dir))
                add_match(child_rel, is_dir);
        }

        if(is_dir)
            search_tree(child_full, child_rel, name, dflag, fflag);
        free(names[i]);
    }
    free(names);

    return;
}

void seek(char* str, char* cur_dir, char* home_dir, char* prev_dir)
{
    int i, j, dflag, eflag, fflag, nd, nf;
    char word[LEN], name[LEN], path[LEN], root[LEN], full[LEN];

    i = 0;
    dflag = 0;
    eflag = 0;
    fflag = 0;
    name[0] = '\0';
    path[0] = '\0';

    while(next_word(str, &i, word))
    {
        if(word[0] == '-' && word[1] != '\0')
        {
            for(j=1 ; word[j] != '\0' ; j++)
            {
                if(word[j] == 'd')
                    dflag = 1;
                else if(word[j] == 'e')
                    eflag = 1;
                else if(word[j] == 'f')
                    fflag = 1;
                else
                {
                    printf(RED "Invalid flags!" RESET "\n");
                    return;
                }
            }
        }
        else if(name[0] == '\0')
            strcpy(name, word);
        else if(path[0] == '\0')
            strcpy(path, word);
    }

    /* -d and -f cannot be used together */
    if(dflag && fflag)
    {
        printf(RED "Invalid flags!" RESET "\n");
        return;
    }
    if(name[0] == '\0')
    {
        printf(RED "Usage: seek <flags> <search> <target_directory>" RESET "\n");
        return;
    }

    if(path[0] == '\0')
        strcpy(path, ".");
    resolve_path(path, root, cur_dir, home_dir, prev_dir);
    if(!directory_exists(root))
    {
        printf("No match found!\n");
        return;
    }

    match_count = 0;
    search_tree(root, ".", name, dflag, fflag);

    if(match_count == 0)
    {
        printf("No match found!\n");
        return;
    }

    nd = 0;
    nf = 0;
    for(i=0 ; i<match_count ; i++)
    {
        if(matches[i].is_dir)
            nd++;
        else
            nf++;
    }

    /* -e only acts on exactly one file or exactly one directory */
    if(eflag && ((nd == 1 && nf == 0) || (nd == 0 && nf == 1)))
    {
        snprintf(full, sizeof(full), "%s/%s", root, matches[0].path + 2);

        if(nf == 1)
        {
            FILE* fptr;
            int ch;

            if(access(full, R_OK) != 0)
            {
                printf(RED "Missing permissions for task!" RESET "\n");
                return;
            }
            printf(GREEN "%s" RESET "\n", matches[0].path);

            fptr = fopen(full, "r");
            if(fptr == NULL)
            {
                printf(RED "Missing permissions for task!" RESET "\n");
                return;
            }
            ch = fgetc(fptr);
            while(ch != EOF)
            {
                putchar(ch);
                ch = fgetc(fptr);
            }
            fclose(fptr);
        }
        else
        {
            char old_dir[LEN];

            if(access(full, X_OK) != 0)
            {
                printf(RED "Missing permissions for task!" RESET "\n");
                return;
            }
            printf(BLUE "%s/" RESET "\n", matches[0].path);

            strcpy(old_dir, cur_dir);
            if(chdir(full) != 0)
            {
                printf(RED "Missing permissions for task!" RESET "\n");
                return;
            }
            strcpy(prev_dir, old_dir);
            if(getcwd(cur_dir, LEN) == NULL)
                printf(RED "Error" RESET "\n");
        }
        return;
    }

    for(i=0 ; i<match_count ; i++)
    {
        if(matches[i].is_dir)
            printf(BLUE "%s" RESET "\n", matches[i].path);
        else
            printf(GREEN "%s" RESET "\n", matches[i].path);
    }

    return;
}
