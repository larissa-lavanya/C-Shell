#include "redirection.h"

/* Splits one pipe stage into command, input file and output file */
int parse_redirect(char* stage, char* command, char* input, char* output, int* append)
{
    int i, j, k;
    char quote, op;
    char* target;

    i = 0;
    j = 0;
    quote = '\0';
    command[0] = '\0';
    input[0] = '\0';
    output[0] = '\0';
    (*append) = 0;

    while(stage[i] != '\0')
    {
        if(quote == '\0' && (stage[i] == '"' || stage[i] == '\''))
            quote = stage[i];
        else if(quote != '\0' && stage[i] == quote)
            quote = '\0';

        if(quote == '\0' && (stage[i] == '<' || stage[i] == '>'))
        {
            op = stage[i];
            i++;
            if(op == '>' && stage[i] == '>')
            {
                (*append) = 1;
                i++;
            }
            while(stage[i] == ' ' || stage[i] == '\t')
                i++;

            if(op == '<')
                target = input;
            else
                target = output;

            k = 0;
            while(stage[i] != '\0' && stage[i] != ' ' && stage[i] != '\t' && stage[i] != '<' && stage[i] != '>')
                target[k++] = stage[i++];
            target[k] = '\0';
            if(k == 0)
                return -1;

            command[j++] = ' ';
        }
        else
            command[j++] = stage[i++];
    }
    command[j] = '\0';

    return 0;
}

/* Runs one stage in the shell itself with its redirections applied */
void redirect(char* stage, char* cur_dir, char* home_dir, char* prev_dir, int background)
{
    char command[LEN], input[LEN], output[LEN];
    int append, fd_in, fd_out, saved_stdin, saved_stdout;

    if(parse_redirect(stage, command, input, output, &append) < 0)
    {
        printf(RED "Invalid redirection" RESET "\n");
        return;
    }

    fflush(stdout);
    saved_stdin = dup(STDIN_FILENO);
    saved_stdout = dup(STDOUT_FILENO);

    if(input[0] != '\0')
    {
        fd_in = open(input, O_RDONLY);
        if(fd_in < 0)
        {
            printf(RED "No such input file found!" RESET "\n");
            close(saved_stdin);
            close(saved_stdout);
            return;
        }

        dup2(fd_in, STDIN_FILENO);
        close(fd_in);
    }

    if(output[0] != '\0')
    {
        if(append)
            fd_out = open(output, O_WRONLY | O_CREAT | O_APPEND, 0644);
        else
            fd_out = open(output, O_WRONLY | O_CREAT | O_TRUNC, 0644);

        if(fd_out < 0)
        {
            dup2(saved_stdin, STDIN_FILENO);
            close(saved_stdin);
            close(saved_stdout);
            printf(RED "Unable to open output file" RESET "\n");
            return;
        }

        dup2(fd_out, STDOUT_FILENO);
        close(fd_out);
    }

    run_command(command, cur_dir, home_dir, prev_dir, background, 0);

    /* Flush before restoring, otherwise buffered output lands on the terminal */
    fflush(stdout);
    dup2(saved_stdin, STDIN_FILENO);
    dup2(saved_stdout, STDOUT_FILENO);
    close(saved_stdin);
    close(saved_stdout);

    return;
}

/* Body of a forked pipe stage, never returns */
static void pipe_stage(char* stage, char* cur_dir, char* home_dir, char* prev_dir)
{
    char command[LEN], input[LEN], output[LEN];
    int append, fd;

    if(parse_redirect(stage, command, input, output, &append) < 0)
    {
        printf(RED "Invalid redirection" RESET "\n");
        fflush(stdout);
        _exit(1);
    }

    if(input[0] != '\0')
    {
        fd = open(input, O_RDONLY);
        if(fd < 0)
        {
            printf(RED "No such input file found!" RESET "\n");
            fflush(stdout);
            _exit(1);
        }
        dup2(fd, STDIN_FILENO);
        close(fd);
    }

    if(output[0] != '\0')
    {
        if(append)
            fd = open(output, O_WRONLY | O_CREAT | O_APPEND, 0644);
        else
            fd = open(output, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if(fd < 0)
        {
            printf(RED "Unable to open output file" RESET "\n");
            fflush(stdout);
            _exit(1);
        }
        dup2(fd, STDOUT_FILENO);
        close(fd);
    }

    run_command(command, cur_dir, home_dir, prev_dir, 0, 1);
    fflush(stdout);
    _exit(0);
}

/* All stages are started together so big outputs cannot block on a full pipe */
void pipes(char* segment, char** stages, int num, char* cur_dir, char* home_dir, char* prev_dir, int background)
{
    int fds[MAX_STAGES][2];
    pid_t pids[MAX_STAGES];
    pid_t pgid;
    int i, j, status, stopped;
    status = 0;
    sigset_t old;
    time_t start;

    for(i=0 ; i<num-1 ; i++)
    {
        if(pipe(fds[i]) < 0)
        {
            printf(RED "Error" RESET "\n");
            for(j=0 ; j<i ; j++)
            {
                close(fds[j][0]);
                close(fds[j][1]);
            }
            return;
        }
    }

    fflush(stdout);
    block_sigchld(&old);
    pgid = 0;
    stopped = 0;
    start = time(NULL);

    for(i=0 ; i<num ; i++)
    {
        pids[i] = fork();
        if(pids[i] == 0)
        {
            /* All stages share one process group so Ctrl-C / Ctrl-Z hit them together */
            setpgid(0, pgid);
            if(i == 0 && !background)
                give_terminal(getpid());
            reset_child_signals();

            if(i > 0)
                dup2(fds[i-1][0], STDIN_FILENO);
            if(i < num - 1)
                dup2(fds[i][1], STDOUT_FILENO);
            for(j=0 ; j<num-1 ; j++)
            {
                close(fds[j][0]);
                close(fds[j][1]);
            }

            pipe_stage(stages[i], cur_dir, home_dir, prev_dir);
        }
        else if(pids[i] > 0)
        {
            if(i == 0)
                pgid = pids[0];
            setpgid(pids[i], pgid);
        }
        else
            printf(RED "Error" RESET "\n");
    }

    for(j=0 ; j<num-1 ; j++)
    {
        close(fds[j][0]);
        close(fds[j][1]);
    }

    if(pgid == 0)
    {
        unblock_sigchld(&old);
        return;
    }

    if(background)
    {
        add_process(pgid, segment, "Running");
        printf("%d\n", pgid);
        fflush(stdout);
        unblock_sigchld(&old);
        return;
    }
    unblock_sigchld(&old);

    give_terminal(pgid);
    fg_pid = pgid;
    for(i=0 ; i<num ; i++)
    {
        if(pids[i] <= 0)
            continue;
        while(waitpid(pids[i], &status, WUNTRACED) < 0 && errno == EINTR)
            ;
        if(WIFSTOPPED(status))
            stopped = 1;
    }
    fg_pid = -1;
    give_terminal(shell_pgid);

    if(stopped)
        add_process(pgid, segment, "Stopped");
    else if((int)(time(NULL) - start) > 2)
    {
        int k;
        k = 0;
        next_word(segment, &k, note_command);
        note_time = (int)(time(NULL) - start);
    }

    return;
}

/* One ';' / '&' separated command: validates pipes then runs it */
void execute(char* segment, char* cur_dir, char* home_dir, char* prev_dir, int background)
{
    char copy[LEN];
    char* stages[MAX_STAGES];
    int num, i;
    char quote;

    strcpy(copy, segment);
    num = 0;
    stages[num++] = copy;
    quote = '\0';

    for(i=0 ; copy[i] != '\0' ; i++)
    {
        if(quote == '\0' && (copy[i] == '"' || copy[i] == '\''))
            quote = copy[i];
        else if(quote != '\0' && copy[i] == quote)
            quote = '\0';
        else if(quote == '\0' && copy[i] == '|')
        {
            if(num >= MAX_STAGES)
            {
                printf(RED "Too many pipes" RESET "\n");
                return;
            }
            copy[i] = '\0';
            stages[num++] = &copy[i+1];
        }
    }

    /* Nothing to the left or right of a pipe */
    for(i=0 ; i<num ; i++)
    {
        trim(stages[i]);
        if(stages[i][0] == '\0')
        {
            printf(RED "Invalid use of pipe" RESET "\n");
            return;
        }
    }

    if(num == 1)
        redirect(stages[0], cur_dir, home_dir, prev_dir, background);
    else
        pipes(segment, stages, num, cur_dir, home_dir, prev_dir, background);

    return;
}
