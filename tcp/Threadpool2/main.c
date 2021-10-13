#include "threadpool.h"
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

void taskFunc(void *arg)
{
	int* num = (int*)arg;
	printf("thread %ld is working, number = %d\n", pthread_self(), *num);
	usleep(10000);
}

int main(void *arg)
{
	ThreadPool* pool = threadPoolCreate(3, 10, 100);
	int i;
	for(i=0; i<100; i++)
	{
		int *num = (int*)malloc(sizeof(int));
		*num = i + 100;
		threadPoolAdd(pool, taskFunc, num);
	}
	sleep(30);

	threadPoolDestroy(pool);
	printf("niah");

	return 0;
}