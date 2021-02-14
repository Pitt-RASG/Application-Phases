#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include <inttypes.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/prctl.h>
#include <linux/limits.h>

static int predictor_input_pipe = -1;
static int predictor_output_pipe = -1;
static int predictor_pid = -1;

static void send_to_predictor(const char* fmt, ...)
{
    char buffer[512];

    va_list va;
    va_start(va, fmt);
    int count = vsnprintf(buffer, sizeof(buffer) - 1, fmt, va);
    va_end(va);

    if (count < 0) 
    {
        perror("Scheduler: failed to vsnprintf during send_to_scheduler");
        abort();
    } 
    else 
    {
        buffer[count++] = '\n';
        buffer[count] = '\0';

        int written = write(predictor_input_pipe, buffer, count);
        if (written == -1)
        {
            perror("Scheduler: failed to write to scheduler");
            abort();
        }
        else if (written != count)
        {
            fprintf(stderr, "Scheduler: count mismatch during send_to_scheduler\n");
            abort();
        }
    }
}

static void recv_from_predictor(const char* fmt, ...)
{
    int count = 0;
    char buffer[512];

    while (count == 0 || buffer[count-1] != '\n')
    {
        int result = read(predictor_output_pipe, buffer, sizeof(buffer) - count);
        if (result <= 0)
        {
            perror("Scheduler: failed to read from scheduler pipe\n");
            abort();
        }

        count += result;
        assert(count < (int) sizeof(buffer) - 1);
    }

    va_list va;
    va_start(va, fmt);
    vsscanf(buffer, fmt, va);
    va_end(va);
}

static int spawn_predictor(const char* command)
{
    int inpipefd[2] = {-1, -1};
    int outpipefd[2] = {-1, -1};

    if (pipe(inpipefd) == -1 || pipe(outpipefd) == -1)
    {
        perror("Scheduler: failed to create scheduling pipes");
        return 0;
    }

    int pid = fork();
    if (pid == -1)
    {
        perror("Scheduler: failed to fork scheduler");
        close(inpipefd[0]);
        close(inpipefd[1]);
        close(outpipefd[0]);
        close(outpipefd[1]);
        return 0;
    }
    else if (pid == 0)
    {
        dup2(outpipefd[0], STDIN_FILENO);
        dup2(inpipefd[1], STDOUT_FILENO);

        close(outpipefd[1]);
        close(inpipefd[0]);

        // receive SIGTERM once the parent process dies
        prctl(PR_SET_PDEATHSIG, SIGTERM);

        // execute command to executue Python script
        execl("/bin/sh", "sh", "-c", command, NULL);
        perror("Scheduler: execl failed");
        return 0;
    }
    else
    {
        close(outpipefd[0]);
        close(inpipefd[1]);
        predictor_pid = pid;
        predictor_input_pipe = outpipefd[1];
        predictor_output_pipe = inpipefd[0];
        return 1;
    }
}

int main(int argc, char* argv[])
{
    int i;
    int pmc1 = 71129;
    int pmc2 = 31704175;
    int pmc3 = 122;
    int pmc4 = 14813781;
    int cluster = 1;

    int predicted_config;

    // start the predictor
    if(!spawn_predictor("python3 ./predictor.py"))
    {
        fprintf(stderr,"Error: spawn predictor\n");        
        return 1;
    }

    i = 0;
    while(i < 10) 
    {
        send_to_predictor("%d,%d,%d,%d,%d", pmc1, pmc2, pmc3, pmc4, cluster);
        recv_from_predictor("%d", &predicted_config);
        fprintf(stderr, "Value received from predictor: %d\n", predicted_config);
        i++;
    }
   
    kill(predictor_pid, SIGTERM);
    return 0;
}
