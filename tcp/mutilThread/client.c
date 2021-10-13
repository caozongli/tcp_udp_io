#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include "wrap.h"
#define SERVER_PORT 8000
#define MAXLINE 4096

int main(int argc, char *argv[])
{
    struct sockaddr_in servaddr;
    char buf[MAXLINE];
    int sockfd, n;

    sockfd = Socket(AF_INET, SOCK_STREAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    inet_pton(AF_INET, "192.168.125.128", &servaddr.sin_addr);
    servaddr.sin_port = htons(SERVER_PORT);

    Connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

    while(fgets(buf, MAXLINE, stdin) != NULL)
    {
        Write(sockfd, buf, strlen(buf));
        n = Read(sockfd, buf, MAXLINE);
        if(n == 0)
        {
            printf("the other side has been closed.\n");
        }
        else
        {
            Write(STDOUT_FILENO, buf, n);
        }

    }
    Close(sockfd);

    return 0;
}
