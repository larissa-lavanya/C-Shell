#include "reveal.h"

int directory_exists(const char *path)
{
    struct stat info;

    if (stat(path, &info) != 0)
        return 0;
    else if (S_ISDIR(info.st_mode))
        return 1;
    return 0;
}

int compare_file(const void* a, const void* b)
{
    int temp;
    temp = strcmp(*((char* const*)a), *((char* const*)b));
    return temp;
}

/* Prints names (sorted already). info = 1 gives the same columns as ls -l */
void list(char* base, char** names, int count, int info)
{
    int i, wl, wo, wg, ws;
    long total;
    struct stat* sts;
    char (*owners)[64];
    char (*groups)[64];
    char full[LEN], tmp[64];
    time_t now;

    sts = (struct stat*)calloc(count + 1, sizeof(struct stat));
    owners = malloc((count + 1) * sizeof(*owners));
    groups = malloc((count + 1) * sizeof(*groups));
    wl = 0;
    wo = 0;
    wg = 0;
    ws = 0;
    total = 0;
    now = time(NULL);

    if(info)
    {
        for(i=0 ; i<count ; i++)
        {
            if(base != NULL)
                snprintf(full, sizeof(full), "%s/%s", base, names[i]);
            else
                snprintf(full, sizeof(full), "%s", names[i]);
            if(lstat(full, &sts[i]) != 0)
                memset(&sts[i], 0, sizeof(struct stat));

            struct passwd *pwd = getpwuid(sts[i].st_uid);
            struct group  *grp = getgrgid(sts[i].st_gid);
            if(pwd != NULL)
                snprintf(owners[i], 64, "%s", pwd->pw_name);
            else
                snprintf(owners[i], 64, "%d", (int)sts[i].st_uid);
            if(grp != NULL)
                snprintf(groups[i], 64, "%s", grp->gr_name);
            else
                snprintf(groups[i], 64, "%d", (int)sts[i].st_gid);

            total += sts[i].st_blocks;
            snprintf(tmp, sizeof(tmp), "%lu", (unsigned long)sts[i].st_nlink);
            if((int)strlen(tmp) > wl)
                wl = strlen(tmp);
            if((int)strlen(owners[i]) > wo)
                wo = strlen(owners[i]);
            if((int)strlen(groups[i]) > wg)
                wg = strlen(groups[i]);
            snprintf(tmp, sizeof(tmp), "%ld", (long)sts[i].st_size);
            if((int)strlen(tmp) > ws)
                ws = strlen(tmp);
        }
        if(base != NULL)
            printf("total %ld\n", total / 2);
    }

    for(i=0 ; i<count ; i++)
    {
        struct stat target;
        const char* color;

        if(base != NULL)
            snprintf(full, sizeof(full), "%s/%s", base, names[i]);
        else
            snprintf(full, sizeof(full), "%s", names[i]);

        /* Blue for directories, green for executables, white for the rest */
        if(stat(full, &target) != 0)
            target = sts[i];
        if(S_ISDIR(target.st_mode))
            color = BLUE;
        else if(target.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH))
            color = GREEN;
        else
            color = WHITE;

        if(info)
        {
            struct stat* s = &sts[i];
            char perm[11], date[64];
            char type;

            if(S_ISDIR(s->st_mode))
                type = 'd';
            else if(S_ISLNK(s->st_mode))
                type = 'l';
            else if(S_ISCHR(s->st_mode))
                type = 'c';
            else if(S_ISBLK(s->st_mode))
                type = 'b';
            else if(S_ISFIFO(s->st_mode))
                type = 'p';
            else if(S_ISSOCK(s->st_mode))
                type = 's';
            else
                type = '-';

            perm[0] = type;
            perm[1] = (s->st_mode & S_IRUSR) ? 'r' : '-';
            perm[2] = (s->st_mode & S_IWUSR) ? 'w' : '-';
            perm[3] = (s->st_mode & S_IXUSR) ? 'x' : '-';
            perm[4] = (s->st_mode & S_IRGRP) ? 'r' : '-';
            perm[5] = (s->st_mode & S_IWGRP) ? 'w' : '-';
            perm[6] = (s->st_mode & S_IXGRP) ? 'x' : '-';
            perm[7] = (s->st_mode & S_IROTH) ? 'r' : '-';
            perm[8] = (s->st_mode & S_IWOTH) ? 'w' : '-';
            perm[9] = (s->st_mode & S_IXOTH) ? 'x' : '-';
            perm[10] = '\0';

            /* Recent files show the time, old ones show the year (like ls) */
            if(now - s->st_mtime > 15778476 || s->st_mtime > now)
                strftime(date, sizeof(date), "%b %e  %Y", localtime(&s->st_mtime));
            else
                strftime(date, sizeof(date), "%b %e %H:%M", localtime(&s->st_mtime));

            printf("%s %*lu %-*s %-*s %*ld %s ", perm, wl, (unsigned long)s->st_nlink, wo, owners[i], wg, groups[i], ws, (long)s->st_size, date);
            printf("%s%s" RESET, color, names[i]);

            if(S_ISLNK(s->st_mode))
            {
                char link[LEN];
                ssize_t len;
                len = readlink(full, link, sizeof(link) - 1);
                if(len > 0)
                {
                    link[len] = '\0';
                    printf(" -> %s", link);
                }
            }
            printf("\n");
        }
        else
            printf("%s%s" RESET "\n", color, names[i]);
    }

    free(sts);
    free(owners);
    free(groups);
    return;
}

void reveal(char* str, char* cur_dir, char* home_dir, char* prev_dir)
{
    int i, j, lflag, aflag, count, cap;
    char word[LEN], path[LEN], target[LEN];
    char** names;
    DIR* dr;
    struct dirent* dir_ptr;
    struct stat info;

    i = 0;
    lflag = 0;
    aflag = 0;
    path[0] = '\0';

    /* Flags can be given in any combination: -a -l, -la, -lala ... */
    while(next_word(str, &i, word))
    {
        if(word[0] == '-' && word[1] != '\0')
        {
            for(j=1 ; word[j] != '\0' ; j++)
            {
                if(word[j] == 'l')
                    lflag = 1;
                else if(word[j] == 'a')
                    aflag = 1;
                else
                {
                    printf(RED "Invalid flags!" RESET "\n");
                    return;
                }
            }
        }
        else if(path[0] == '\0')
            strcpy(path, word);
    }

    /* No path means the current directory */
    if(path[0] == '\0')
        strcpy(path, ".");
    resolve_path(path, target, cur_dir, home_dir, prev_dir);

    if(stat(target, &info) != 0)
    {
        printf(RED "Invalid path" RESET "\n");
        return;
    }

    /* A file is just listed by itself */
    if(!S_ISDIR(info.st_mode))
    {
        char* single[1];
        single[0] = target;
        list(NULL, single, 1, lflag);
        return;
    }

    dr = opendir(target);
    if(dr == NULL)
    {
        printf(RED "Error" RESET "\n");
        return;
    }

    names = NULL;
    count = 0;
    cap = 0;
    dir_ptr = readdir(dr);
    while(dir_ptr != NULL)
    {
        if(aflag || dir_ptr->d_name[0] != '.')
        {
            if(count == cap)
            {
                if(cap == 0)
                    cap = 16;
                else
                    cap = cap * 2;
                names = (char**)realloc(names, cap * sizeof(char*));
            }
            names[count] = strdup(dir_ptr->d_name);
            count++;
        }
        dir_ptr = readdir(dr);
    }
    closedir(dr);

    qsort(names, count, sizeof(char*), compare_file);
    list(target, names, count, lflag);

    for(i=0 ; i<count ; i++)
        free(names[i]);
    free(names);

    return;
}
