#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <errno.h>
#include "wrap.h"

#define MAXLINE 80
#define SERVER_PORT 8000
#define OPEN_MAX 1024

int main(int argc, char* argv[])
{
	int i, j, maxi, listenfd, connfd, sockfd;
	int nready, efd, res;
	ssize_t n;
	char buf[MAXLINE], str[INET_ADDRSTRLEN];
	socklen_t clilen;
	int client[OPEN_MAX];
	struct sockaddr_in cliaddr, servaddr;
	struct epoll_event tep, ep[OPEN_MAX];

	listenfd = Socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	// inet_pton(AF_INET, "192.168.125.128", &servaddr.sin_addr);
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(SERVER_PORT);

	Bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr));

	Listen(listenfd, 20);

	for(i = 0; i < OPEN_MAX; i++)
		client[i] = -1;
	maxi = -1;

	efd = Epoll_create(OPEN_MAX);

	tep.events = EPOLLIN;
	tep.data.fd = listenfd;
	res = Epoll_ctl(efd, EPOLL_CTL_ADD, listenfd, &tep);

	for(; ; )
	{
		nready = Epoll_wait(efd, ep, OPEN_MAX, -1); 	/*阻塞监听*/
		for(i = 0; i < nready; i++)
		{
			if(!(ep[i].events & EPOLLIN))
				continue;
			if(ep[i].data.fd == listenfd)
			{
				clilen = sizeof(cliaddr);
				connfd = Accept(listenfd, (struct sockaddr*)&cliaddr, &clilen);
				printf("recived from %s at Port %d\n", inet_ntop(AF_INET, &cliaddr.sin_addr, str, sizeof(str)), ntohs(cliaddr.sin_port));

				for( j = 0; j < OPEN_MAX; j++)
				{
					if(client[j] == -1)
					{
						client[j] = connfd;
						break;
					}
				}	
				
				if(j == OPEN_MAX)
					perr_exit("too many clients");
				
				if(j > maxi)
					maxi = j;
				tep.events = EPOLLIN; tep.data.fd = connfd;
				res = Epoll_ctl(efd, EPOLL_CTL_ADD, connfd, &tep);
			}
			else
			{
				sockfd = ep[i].data.fd;
				n = Read(sockfd, buf, MAXLINE);
				if(n == 0)
				{
					for(j = 0; j <= maxi; j++)
					{
						if(client[j] == sockfd)
						{
							client[j] = -1;
							break;
						}
					}
					res = Epoll_ctl(efd, EPOLL_CTL_DEL, sockfd, NULL);
					Close(sockfd);
					printf("client[%d] closed connection\n", j);
				}
				else
				{
					for( j = 0; j < n; j++)
					{
						buf[j] = toupper(buf[j]);
					}
					Write(sockfd, buf, n);
				}
			}
		}
	}
	close(listenfd);
	close(efd);

	return 0;
}

