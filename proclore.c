#include "proclore.h"

void proclore_print(pid_t pid, char* home_dir)
{
    char path[LEN], buffer[LEN], line[LEN];
    FILE* fptr;
    int ppid, pgid, session, tty, tpgid;
    unsigned long vsize;
    char state, *p;

    snprintf(path, sizeof(path), "/proc/%d/stat", pid);
    fptr = fopen(path, "r");
    if (fptr == NULL)
    {
        printf(RED "Invalid PID" RESET "\n");
        return;
    }
    if(fgets(line, sizeof(line), fptr) == NULL)
    {
        fclose(fptr);
        printf(RED "Invalid PID" RESET "\n");
        return;
    }
    fclose(fptr);

    /* The process name can contain spaces, so parsing starts after the last ')' */
    p = strrchr(line, ')');
    if(p == NULL || sscanf(p + 2, "%c %d %d %d %d %d", &state, &ppid, &pgid, &session, &tty, &tpgid) != 6)
    {
        printf(RED "Invalid PID" RESET "\n");
        return;
    }

    /* Virtual memory in KB */
    vsize = 0;
    snprintf(path, sizeof(path), "/proc/%d/status", pid);
    fptr = fopen(path, "r");
    if(fptr != NULL)
    {
        while(fgets(line, sizeof(line), fptr) != NULL)
        {
            if(strncmp(line, "VmSize:", 7) == 0)
            {
                sscanf(line + 7, "%lu", &vsize);
                break;
            }
        }
        fclose(fptr);
    }

    snprintf(path, sizeof(path), "/proc/%d/exe", pid);
    ssize_t len = readlink(path, buffer, sizeof(buffer) - 1);
    if (len != -1)
        buffer[len] = '\0';
    else
        strcpy(buffer, "Unknown");

    /* Paths inside the home directory are shown with ~ */
    int home_len;
    home_len = strlen(home_dir);
    if(strncmp(buffer, home_dir, home_len) == 0 && (buffer[home_len] == '/' || buffer[home_len] == '\0'))
    {
        char temp[LEN];
        snprintf(temp, sizeof(temp), "~%s", buffer + home_len);
        strcpy(buffer, temp);
    }

    /* '+' means the process is in the foreground process group */
    char fg_bg_status[2];
    fg_bg_status[0] = (pgid == tpgid) ? '+' : '\0';
    fg_bg_status[1] = '\0';

    printf("pid : %d\n", pid);
    printf("process status : %c%s\n", state, fg_bg_status);
    printf("Process Group : %d\n", pgid);
    printf("Virtual memory : %lu\n", vsize);
    printf("executable path : %s\n", buffer);

    return;
}

void proclore(char* str, char* home_dir)
{
    int i;
    char word[LEN];
    pid_t pid;

    i = 0;
    if(!next_word(str, &i, word))
        pid = getpid();
    else if(is_number(word))
        pid = atoi(word);
    else
    {
        printf(RED "Invalid PID" RESET "\n");
        return;
    }

    proclore_print(pid, home_dir);
    return;
}
