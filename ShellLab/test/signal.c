#define _GNU_SOURCE

#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

extern char **environ;

// int kill(pid_t pid, int sig);

void sigint_handler(int sig)
{
    printf("Ctrl + C\n");
}

void sigchld_handler(int sig)
{
    pid_t pid;
    int stat;
    char string[60] = {0};
    const char *s = "empty";
    if ((pid = waitpid(-1, &stat, 0)) > 0)
    {
        if (WIFEXITED(stat))
            write(STDOUT_FILENO, "chld exit\n", 11);
    }
}

void sigstp_handler(int sig){

    // 进入函数后， 由于还未处理完成SIGTSTP
    // 所以后续的信号应该是被阻塞的
    // 因此为了调用 SIGTSTP 的 SIG_DFL
    // 需要先接触阻塞

    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGTSTP);
    sigprocmask(SIG_UNBLOCK, &mask, NULL);

    signal(SIGTSTP, SIG_DFL);

    kill(getpid(), SIGTSTP);

    signal(SIGTSTP, sigstp_handler);

}

int main()
{
    pid_t pid;
    // int i = 1;
    struct timeval start, end;

    signal(SIGTSTP, sigstp_handler);
    signal(SIGCHLD, sigchld_handler);
    
    if ((pid = fork()) == 0)
    {
        setpgid(0, 0);
        printf("i am in child process\n");
        char *argv[] = {"./myspin", "30", NULL};
        execve(argv[0], argv, environ);
    }
   
    gettimeofday(&start, NULL);
    sleep(2);
    printf("i am in parent process,my pid = %d, child pid = %d\n", getpid(), pid);
    gettimeofday(&end, NULL);
    printf("parent process over, time = %ld\n", end.tv_sec - start.tv_sec);
    return 0;
}