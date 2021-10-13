#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#define SERVER_PORT 8000
#define MAXLINE 80 
  
int main(void)
{
    struct sockaddr_in serveraddr;
    char buf[MAXLINE];
    socklen_t serveraddr_len;

    char ipstr[] = "192.168.125.128";

    int confd;
    //1.创建一个socket
    confd = socket(AF_INET, SOCK_DGRAM, 0);

    //2.初始化服务器地址
    bzero(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    //"192.168.125.128"
    inet_pton(AF_INET, ipstr, &serveraddr.sin_addr.s_addr);
    serveraddr.sin_port = htons(SERVER_PORT);

    //3.向服务器发送数据
    while(fgets(buf, MAXLINE, stdin) != NULL)
    {
        n = sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr *)&serveraddr, sizeof(serveraddr));

        n = recvfrom(sockfd, buf, MAXLINE, 0, NULL, 0);
        write(STDOUT_FILENO, buf, n);
    }

    //4.关闭socket
    close(confd);

    return 0;
}
