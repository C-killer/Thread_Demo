#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

#include "threadPool.h"

void taskFunc(void* arg){
    int num = *(int*) arg;
    printf("thread %ld is working, number = %d\n",
        pthread_self(), num);
    sleep(1);
}


int main() {
    // 创建线程池
    ThreadPool* pool = createThreadPool(10, 10, 100);
    for (int i=0;i<100;i++) {
        int* n = (int *)(malloc(sizeof(int)));
        *n = i + 100;
        threadPoolAdd(pool, taskFunc, n);
    }
    pthread_join(pthread_self(), NULL);
    threadPoolDestroy(pool);
    printf("======= END =======\n");
    return 0;
}