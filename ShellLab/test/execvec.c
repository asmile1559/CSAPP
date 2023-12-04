#include <stdio.h>
#include "unistd.h"

extern char **environ;

int main()
{
    char *argv[] = {"/bin/ls", "-l", "-a", NULL};
    if(execve(argv[0], argv, environ) < 0)
    {
        printf("command not found !");
    }
    return 0;
}