#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <pthread.h>
#include "wrap.h"

#define SERVER_PORT 8000
#define MAXLINE 80
struct s_info
{
    struct sockaddr_in cliaddr;
    int connfd;
};

void *do_work(void *arg)
{
    int n, i;
    struct s_info *ts = (struct s_info*)arg;
    char buf[MAXLINE];
    char str[INET_ADDRSTRLEN];
    printf("pid=%d\n", getpid());

    /*可以在创建线程前设置线程创建属性，哪种效率更高？*/
    //pthread_detach(pthread_self());
    while(1)
    {
        n = Read(ts->connfd, buf, MAXLINE);
        if(n == 0)
        {
            printf("the other side has been closed.\n");
            break;
        }
        printf("recerved from %s at PORT %d\n",
                inet_ntop(AF_INET, &(*ts).cliaddr.sin_addr, str, sizeof(str)),
                ntohs((*ts).cliaddr.sin_port));
        for(i = 0; i < n; i++)
        {
            buf[i] = toupper(buf[i]);
        }
        Write(ts->connfd, buf, n);
    }
    Close(ts->connfd);

}

int main(void)
{
    struct sockaddr_in servaddr, cliaddr;
    socklen_t cliaddr_len;
    int listenfd, connfd;
    int i = 0;
    pthread_t tid;
    pthread_attr_t attr; //里面还是垃圾值

    pthread_attr_init(&attr);   //attr里面保存创建线程的默认属性
    /*int detachstate: PTHREAD_CREATE_DETACHED, PTHREAD_CREATE_JOINABLE*/
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);    //设置线程分离属性

    struct s_info ts[383];

    listenfd = Socket(AF_INET, SOCK_STREAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERVER_PORT);

    Bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

    Listen(listenfd, 20);

    printf("Accepting connections ...\n");
    while(1)
    {
        cliaddr_len = sizeof(cliaddr);
        connfd = Accept(listenfd, (struct sockaddr *)&cliaddr, &cliaddr_len);
        ts[i].cliaddr = cliaddr;
        ts[i].connfd = connfd;
        /*达到线程最大数时，pthread_create出错处理，增加服务器稳定性*/
        pthread_create(&tid, &attr, do_work, (void*)&ts[i]);
        i++;
    }

    return 0;
}
