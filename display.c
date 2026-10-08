#include "display.h"

int display()
{
    char username[LEN], sysname[LEN], home_dir[LEN], cur_dir[LEN], prev_dir[LEN];
    char str[LEN], note[LEN];
    struct passwd* pwd;

    /* getlogin_r fails without a terminal, so the username comes from the uid */
    pwd = getpwuid(getuid());
    if(pwd == NULL || gethostname(sysname, sizeof(sysname)) != 0 || getcwd(home_dir, sizeof(home_dir)) == NULL)
    {
        printf(RED "Error" RESET "\n");
        return 1;
    }
    strcpy(username, pwd->pw_name);
    strcpy(cur_dir, home_dir);
    strcpy(prev_dir, home_dir);

    snprintf(log_path, sizeof(log_path), "%s/.shell_log", home_dir);
    load_myshrc(home_dir);

    while(1)
    {
        /* Time taken by the last foreground process goes into this prompt */
        note[0] = '\0';
        if(note_command[0] != '\0')
        {
            snprintf(note, sizeof(note), " %s : %ds", note_command, note_time);
            note_command[0] = '\0';
        }

        int home_len;
        home_len = strlen(home_dir);
        if(strncmp(cur_dir, home_dir, home_len) == 0 && (cur_dir[home_len] == '/' || cur_dir[home_len] == '\0'))
            printf(GREEN "<%s@%s" WHITE ":" BLUE "~%s%s> " RESET, username, sysname, cur_dir + home_len, note);
        else
            printf(GREEN "<%s@%s" WHITE ":" BLUE "%s%s> " RESET, username, sysname, cur_dir, note);
        fflush(stdout);

        if(fgets(str, sizeof(str), stdin) == NULL)
        {
            /* Ctrl-D */
            if(feof(stdin) || ferror(stdin))
            {
                printf("\n");
                Ctrl_D();
            }
            clearerr(stdin);
            continue;
        }

        /* Throw away the rest of a line that is too long */
        if(strchr(str, '\n') == NULL && !feof(stdin))
        {
            int ch;
            ch = getchar();
            while(ch != '\n' && ch != EOF)
                ch = getchar();
        }

        trim(str);
        if(str[0] == '\0')
            continue;

        log_store(str);
        tokenize(str, cur_dir, home_dir, prev_dir);
    }

    return 0;
}
