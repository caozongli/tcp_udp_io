#include "threadpool.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

const int NUMBER = 2;
//任务结构体
typedef struct Task
{
	void(*func)(void *arg);
	void *arg;
}Task;

//线程池结构
struct ThreadPool
{
	Task *task;				//任务队列
	int queueCapacity;		//任务队列容量
	int queueSize;			//现存任务
	int queueFront;			//队头 -> 取数据
	int queueRear;			//队尾 -> 放数据

	pthread_t managerID;	//管理者线程ID
	pthread_t *threadIDs;	//工作线程ID
	int minNum;				//最小工作线程数
	int maxNum;				//最大工作线程数
	int liveNum;			//存活线程数
	int busyNum;			//忙线程数
	int exitNum;			//要销毁的线程数

	pthread_mutex_t mutexPool;	//锁住整个线程池
	pthread_mutex_t mutexBusy;	//锁住busyNum变量
	pthread_cond_t notFull;		//任务队列是否满
	pthread_cond_t notEmpty;	//任务队列是否空
	
	int shutdown;				//是否要销毁线程池，销毁为1，不销毁0
};

//创建线程池并初始化
ThreadPool* threadPoolCreate(int min, int max, int queueSize)
{
	ThreadPool* pool = (ThreadPool*)malloc(sizeof(ThreadPool));
	do
	{
		if(pool == NULL)
		{
			perror("malloc threadpool fail...\n");
			break;
		}
		bzero(pool, sizeof(ThreadPool));

		pool->task = (Task*)malloc(sizeof(Task)*queueSize);
		if(pool->task == NULL)
		{
			perror("malloc task fail...\n");
			break;
		}
		// memset(&pool->task, 0, sizeof(Task*)*queueSize);
		pool->queueCapacity = queueSize;
		pool->queueSize = 0;
		pool->queueFront = 0;
		pool->queueRear = 0;

		pool->threadIDs = (pthread_t*)malloc(sizeof(pthread_t)*max);
		if(pool->threadIDs == NULL)
		{
			perror("malloc threadIDs fail...\n");
			break;
		}
		pool->maxNum = max;
		pool->minNum = min;
		pool->liveNum = 0;
		pool->busyNum = 0;
		pool->exitNum = 0;

		if(pthread_mutex_init(&pool->mutexPool, NULL) ||
		pthread_mutex_init(&pool->mutexBusy, NULL) ||
		pthread_cond_init(&pool->notEmpty, NULL) || 
		pthread_cond_init(&pool->notFull, NULL)
		)
		{
			perror("mutex or condition init fail...\n");
			break;
		}

		pthread_create(&pool->managerID, NULL, manager, pool);
		int i;
		for(i=0; i<min; i++)
		{
			pthread_create(&pool->threadIDs[i], NULL, worker, pool);
		}
		pool->liveNum = min;
		return pool;
	}while (0);

	//释放
	if(pool && pool->task) free(pool->task);
	if(pool && pool->threadIDs) free(pool->threadIDs);
	if(pool) free(pool);
	return NULL;
}

//添加任务
void threadPoolAdd(ThreadPool* pool, void(*func)(void*), void* arg)
{
	pthread_mutex_lock(&pool->mutexPool);
	while(pool->queueSize == pool->queueCapacity && !pool->shutdown)
	{
		//阻塞生产者线程
		pthread_cond_wait(&pool->notFull, &pool->mutexPool);
	}
	if(pool->shutdown)
	{
		pthread_mutex_unlock(&pool->mutexPool);
		return;
	}

	//添加任务
	pool->task[pool->queueRear].func = func;
	pool->task[pool->queueRear].arg = arg;
	pool->queueRear = (pool->queueRear + 1) % pool->queueCapacity;
	pool->queueSize++;

	pthread_mutex_unlock(&pool->mutexPool);
	pthread_cond_signal(&pool->notEmpty);
}


//销毁线程池
int threadPoolDestroy(ThreadPool* pool)
{
	if(pool == NULL) return -1;

	//关闭线程池
	pool->shutdown = 1;
	//阻塞回收管理者线程
	pthread_join(pool->managerID, NULL);
	//唤醒阻塞的消费者
	int i;
	for(i=0; i<pool->liveNum; i++)
	{
		pthread_cond_signal(&pool->notEmpty);
	}

	//释放内存堆
	if(pool->task) free(pool->task);
	if(pool->threadIDs) free(pool->threadIDs);

	pthread_mutex_destroy(&pool->mutexPool);
	pthread_mutex_destroy(&pool->mutexBusy);
	pthread_cond_destroy(&pool->notEmpty);
	pthread_cond_destroy(&pool->notFull);

	if(pool) free(pool);
	
	return 0;
}


//获取线程池中忙的线程个数
int threadPoolBusyNum(ThreadPool* pool)
{
	pthread_mutex_lock(&pool->mutexBusy);
	int busyNum = pool->busyNum;
	pthread_mutex_unlock(&pool->mutexBusy);
	return busyNum;
}


//获取线程池中存活的线程的个数
int threadPoolAliveNum(ThreadPool* pool)
{
	pthread_mutex_lock(&pool->mutexPool);
	int liveNum = pool->liveNum;
	pthread_mutex_unlock(&pool->mutexPool);
	return liveNum;
}


void* worker(void* arg)
{
	ThreadPool* pool = (ThreadPool*)arg;

	while(1)
	{
		pthread_mutex_lock(&pool->mutexPool);
		while(pool->queueSize == 0 && !pool->shutdown)
		{
			//阻塞工作线程
			pthread_cond_wait(&pool->notEmpty, &pool->mutexPool);
			
			//判断是否要销毁线程
			if (pool->exitNum > 0)
			{
				pool->exitNum--;
				if(pool->liveNum > pool->minNum)
				{
					pool->liveNum--;
					pthread_mutex_unlock(&pool->mutexPool);
					threadExit(pool);
				}
			}
		}

		if(pool->shutdown > 0)
		{
			pthread_mutex_unlock(&pool->mutexPool);
			threadExit(pool);
		}

		//从任务队列中取出一个任务
		Task task;
		task.func = pool->task[pool->queueFront].func;
		task.arg = pool->task[pool->queueFront].arg;
		//移动头结点
		pool->queueFront = (pool->queueFront + 1) % pool->queueCapacity;
		pool->queueSize--;

		pthread_cond_signal(&pool->notFull);
		pthread_mutex_unlock(&pool->mutexPool);

		pthread_mutex_lock(&pool->mutexBusy);
		pool->busyNum++;
		pthread_mutex_unlock(&pool->mutexBusy);

		//task.func(task.arg);;
		(*task.func)(task.arg);
		task.arg = NULL;

		printf("thread %ld end working...\n", pthread_self());
		pthread_mutex_lock(&pool->mutexBusy);
		pool->busyNum--;
		pthread_mutex_unlock(&pool->mutexBusy);
	}
	
	return NULL;
}


void* manager(void* arg)
{
	ThreadPool* pool = (ThreadPool*)arg;
	while(!pool->shutdown)
	{
		sleep(3);
		//取出线程池中任务的数量和当前线程的数量
		pthread_mutex_lock(&pool->mutexPool);
		int queueSize = pool->queueSize;
		int liveNum = pool->liveNum;
		pthread_mutex_unlock(&pool->mutexPool);

		//取出忙线程的数量
		pthread_mutex_lock(&pool->mutexBusy);
		int busyNum = pool->busyNum;
		pthread_mutex_unlock(&pool->mutexBusy);

		//添加线程
		//任务个数 > 存活线程个数 && 存活线程个数 < 最大线程数
		if(queueSize > liveNum && liveNum < pool->maxNum)
		{
			pthread_mutex_lock(&pool->mutexPool);
			int counter = 0, i;
			for(i=0; i < pool->maxNum && counter < NUMBER && pool->liveNum < pool->maxNum; ++i)
			{
				if(pool->threadIDs[i] == 0)
				{
					pthread_create(&pool->threadIDs[i], NULL, worker, pool);
					counter++;
					liveNum++;
					pool->liveNum++;
				}
			}
			pthread_mutex_unlock(&pool->mutexPool);
		}

		//销毁线程
		//忙线程*2 < 存活线程 && 存活线程 > 最小线程数
		if(busyNum*2 < liveNum && liveNum > pool->minNum)
		{
			pthread_mutex_lock(&pool->mutexPool);
			pool->exitNum = NUMBER;
			pthread_mutex_unlock(&pool->mutexPool);
			//让工作的线程自杀
			int i;
			for(i=0; i<NUMBER; ++i)
			{
				pthread_cond_signal(&pool->notEmpty);
			}
		}
		//唤醒线程
		//未执行任务个数queueSize > 0 && 忙的线程数 < 存活的线程个数
		if(queueSize > 0 && busyNum < liveNum)
		{
			pthread_cond_signal(&pool->notEmpty);
		}
	}
}


void threadExit(ThreadPool* pool)
{
	pthread_t tid = pthread_self();
	int i;
	for(i=0; i<pool->maxNum; ++i)
	{
		if(pool->threadIDs[i] == tid)
		{
			pool->threadIDs[i] = 0;
			printf("threadExit() called, %ldexiting...\n", tid);
			break;
		}
	}
	pthread_exit(NULL);
}
