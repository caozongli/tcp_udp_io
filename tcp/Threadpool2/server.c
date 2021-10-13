#include "threadpool.h"
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/socket.h>
#include <bits/sockaddr.h>
#include <netinet/in.h>
#include "wrap.h"
#include <string.h>
#include <arpa/inet.h>
#include <ctype.h>

#define SERVER_PORT 8000
#define MAXLINE 80

struct s_info
{
	struct sockaddr_in clientaddr;
	int confd;
};

void connectOfclient(void* arg)
{
	struct s_info* ts = (struct s_info*)arg;
	char buf[MAXLINE];
	char ipstr[128];
	int i, n;

	while(1)
	{
		n = Read(ts->confd, buf, MAXLINE);	
		if(n == 0)
		{
			printf("the other side %s:%d has been closed.\n", 
					inet_ntop(AF_INET, &ts->clientaddr.sin_addr.s_addr, ipstr, sizeof(ipstr)),
					ntohs(ts->clientaddr.sin_port));
			break;
		}
		for(i = 0; i < n; ++i)
		{
			buf[i] = toupper(buf[i]);
		}
		Write(ts->confd, buf, n);
	}
	Close(ts->confd);
}

int main(void* arg)
{
	struct sockaddr_in serveraddr, clientaddr;
	socklen_t clientaddr_len;
	
	int listenfd, connfd;
	int i = 0;
	struct s_info ts[300];
	
	pthread_attr_t attr;	//此时还是垃圾值
	pthread_attr_init(&attr);	//attr里面保存创建线程的默认属性
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);	//设置分离属性

	listenfd = Socket(AF_INET, SOCK_STREAM, 0);

	bzero(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVER_PORT);

	Bind(listenfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr));

	Listen(listenfd, 20);

	printf("Accepting connection...\n");
	ThreadPool* pool = threadPoolCreate(3, 10, 300);
	while(1)
	{
		clientaddr_len = sizeof(clientaddr);
		connfd = Accept(listenfd, (struct sockaddr*)&clientaddr, &clientaddr_len);
		ts[i].clientaddr = clientaddr;
		ts[i].confd = connfd;
		threadPoolAdd(pool, connectOfclient, &ts[i]);
		i++;
	}

	return 0;
}