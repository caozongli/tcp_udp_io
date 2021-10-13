#ifndef _THREADPOOL_H_
#define _THREADPOOL_H_

typedef struct ThreadPool ThreadPool;

//线程池的创建和初始化
ThreadPool* threadPoolCreate(int min, int max, int queueSize);

//销毁线程池
int threadPoolDestroy(ThreadPool* pool);

//给线程池添加任务
void threadPoolAdd(ThreadPool* pool, void(*func)(void*), void* arg);

//获取线程池中忙的线程个数
int threadPoolBusyNum(ThreadPool* ThreadPool);

//获取线程池中存活的线程的个数
int threadPoolAliveNum(ThreadPool* pool);

//////////////
void* worker(void* arg);
void* manager(void* arg);
void threadExit(ThreadPool* pool);



#endif
