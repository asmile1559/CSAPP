#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

extern char **environ;

int main()
{
    pid_t pid;

    char *argv[] = {"/bin/ls", "-l", "-a", NULL};

    if ((pid = fork()) == 0)
    {
        printf("child process in\n");
        
        for (int i = 0; i < 70; i++)
        {
            printf("%d, ", i);
            sleep(1);
            fflush(stdout);
        }
        printf("child process exit\n");
        return 0;
    }

    printf("%d\n", pid);
    waitpid(pid, NULL, WNOHANG);
    printf("nohang!\n");
    waitpid(pid, NULL, WUNTRACED);
    printf("father process\n");
    
    return 1;
}