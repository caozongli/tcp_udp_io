


// 1.单线程

void func(){
	read();
	send();
}

//多个线程共用一个fd

可以 一个线程 用多个fd
不可以 多个线程 用一个fd

// 7*24
// 

while(1){
	int nready = epoll_wait();
	for (int i = 0; i < nready; i++){
		if(epollin){
			int ret = 0;
#if ET
			while(ret == -1)
				ret = read(event[i].data.fd, buffer, length, 0);
			
			// push_to_workqueue();
			query_sql(); //

			loginfo();
#else
			非阻塞fd，一个线程
			ret = read(event[i].data.fd, buffer, length, 0);
#endif
		}
	}
}



网络io多线程做法
1. 单线程处理accept， 多线程处理recv/send --> 1:N

2. 多线程处理accept， 多线程处理recv/send --> M:N
   多个线程处理用一个listenfd，LT，水平触发
   1. ET/LT

3. 多线程 epoll_wait(), 不区分accept与recv/send，来支持多核


多进程，session独立，前后不关联

1. 多进程epoll_wait(), 不区分accept与recv/send， nginx


2. master --> accept，worker --> recv/send， 理论上行的通
进程a，里面 fd=5，
进程b，里面 fd=5；







对于多线程来说，监听一个端口，listenfd

/*错误
void *cb(void *arg){
	listen(); 
}
*/

listen(fd);
pthread_create(thid, NULL, cb, &fd);
pthread_create(thid, NULL, cb, &fd);
pthread_create(thid, NULL, cb, &fd);
pthread_create(thid, NULL, cb, &fd);


reactor，不用跟多线程与多进程联系。
  ：reactor 封装好的epoll

  ：fd，recvbuff，sendbuffer





