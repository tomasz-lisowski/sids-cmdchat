#include "cmdchat.h"
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#define DS_BIN "SIDedicatedServer.x86_64"
#define DS_PATH_SCRIPT "./script"

static int32_t ds_pid = 0;

void sigint_handler(int32_t signum)
{
    printf("Got SIGINT (=%i)\n", signum);
    if (signum == SIGINT)
    {
        if (ds_pid > 0)
        {
            kill(ds_pid, SIGINT);
            printf("Waiting for DS to quit\n");
            waitpid(ds_pid, NULL, WUNTRACED);
            printf("DS has quit\n");
            exit(EXIT_SUCCESS);
        }
    }
}

int32_t ds_run()
{
    char *const ds_name = "./" DS_BIN;
    char *const ds_argv[] = {DS_BIN,     "-config",    "serverconfig.txt",
                             "-console", "-batchmode", "-nographics",
                             "-logFile", NULL};
    int32_t ret = EXIT_FAILURE;

    // Pipe for getting STDOUT of DS in parent.
    int32_t fd[2u] = {0u};
    if (pipe(fd))
    {
        perror("pipe");
        return ret;
    }

    ds_pid = fork();
    if (ds_pid <= -1)
    {
        // Cleanup pipe.
        close(fd[0u]);
        close(fd[1u]);

        perror("fork");
        return ret;
    }

    if (ds_pid == 0u)
    {
        // Child.

        // Close the end of pipe owned by parent.
        close(fd[0u]);

        // Make fd[1] the STDOUT and STDERR of child.
        dup2(fd[1u], STDOUT_FILENO);
        dup2(fd[1u], STDERR_FILENO);
        close(fd[1u]);

        if (execv(ds_name, ds_argv) == -1)
        {
            perror("execv");
            return ret;
        }
    }
    else
    {
        // Parent.

        // Close the end of pipe owned by child.
        close(fd[1u]);

        char buffer[2048u];
        bool running = true;
        while (running)
        {
            // Read from STDOUT of child.
            const ssize_t bytes_read = read(fd[0u], buffer, sizeof(buffer));
            if (bytes_read < 0u)
            {
                perror("read");
                break;
            }
            else if (bytes_read == 0u)
            {
                // DS probably crashed.
                printf("Got EOF from STDOUT of DS\n");
                running = false;
                ret = EXIT_FAILURE;
                break;
            }
            else
            {
                printf("CMDChat (%4u): %.*s\n", (uint32_t)bytes_read,
                       (uint32_t)bytes_read, buffer);
                /* Safe cast to uint32_t due to length arg passed to read. */
                cmdchat_et cmd =
                    cmdchat_parse_raw(buffer, (uint32_t)bytes_read);

                switch (cmd)
                {
                case CMDCHAT_RESTART:
                    running = false;
                    ret = EXIT_SUCCESS;
                    break;
                case CMDCHAT_BACKUP:
                    system("/bin/bash"
                           " " DS_PATH_SCRIPT "/"
                           "sids-backup.sh");
                    break;
                case CMDCHAT_UNKNOWN:
                    break;
                }
            }
        }

        // Kill child to avoid zombie servers.
        kill(ds_pid, SIGINT);

        // Cleanup FDs.
        close(fd[0u]);

        // Just to be sure child has shutdown.
        waitpid(ds_pid, NULL, WUNTRACED);
    }
    return ret;
}

int32_t main()
{
    signal(SIGINT, sigint_handler);
    while (1u)
    {
        ds_pid = 0; // Reset DS PID
        if (ds_run() != EXIT_SUCCESS)
        {
            break;
        };
    }
    return EXIT_SUCCESS;
}
