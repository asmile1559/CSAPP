#define _GNU_SOURCE

#include "csapp.h"

int main(int argc, char **argv)
{
    int fd;
    
    if(argc != 3)
    {
        fprintf(stderr, "useage: %s <host> <port> /n", argv[0]);
        exit(1);
    }

    if ((fd = Open_clientfd(argv[1], argv[2])) < 0)
    {
        printf("open client fd error\n");
        exit(1);
    }

    rio_t rio;
    char buff[1024];
    Rio_readinitb(&rio, fd);
    while(fgets(buff, 1024, stdin)!=NULL)
    {
        Rio_writen(fd, buff, 1024);
        Rio_readlineb(&rio, buff, 1024);
        Fputs(buff, stdout);
    }
    Close(fd);
    return 0;
}