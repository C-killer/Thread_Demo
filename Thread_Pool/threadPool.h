#ifndef THREAD_POOL_H_
#define THREAD_POOL_H_

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

typedef struct _threadPool ThreadPool;

// 创建并初始化线程池
ThreadPool* createThreadPool(int queueCapacity, int minNum, int maxNum);

// 销毁线程池
int threadPoolDestroy(ThreadPool* pool);

// 添加任务到线程池
void threadPoolAdd(ThreadPool* pool, void(*fonc)(void*), void* arg);

// 当前正在工作的线程个数
int threadPoolBusyNum(ThreadPool* pool);

// 当前活着的线程个数
int threadPoolLiveNum(ThreadPool* pool);

void* worker(void* arg);
void* manager(void* arg);

// 退出线程并将线程ID列表中对应的值还原成0
void threadExit(ThreadPool* pool);

#endif  // THREAD_POOL_H_