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
    struct sockaddr_in serveraddr;
    char ipstr[] = "192.168.125.128";
    char buf[MAXLINE];

    int confd, len;
    //1.创建一个socket
    confd = socket(AF_INET, SOCK_STREAM, 0);

    //2.初始化服务器地址
    bzero(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    //"192.168.125.128"
    inet_pton(AF_INET, ipstr, &serveraddr.sin_addr.s_addr);
    serveraddr.sin_port = htons(SERVER_PORT);

    //3.链接服务器
    connect(confd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));

    //4.请求服务器处理数据
    while(fgets(buf, sizeof(buf), stdin))
    {
        write(confd, buf, strlen(buf));
        len = read(confd, buf, sizeof(buf));
        write(STDOUT_FILENO, buf, len);
    }


    //5.关闭socket
    close(confd);

    return 0;
}
