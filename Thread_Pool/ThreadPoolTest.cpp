#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>

#include "ThreadPool.hpp"

/* g++ ThreadPoolTest.cpp -o ThreadPool -lpthread */

void taskFunc(void* arg){
    int num = *(int*) arg;
    printf("thread %ld is working, number = %d\n",
        pthread_self(), num);
    sleep(1);
}



int main() {
    // 创建线程池
    ThreadPool<int> pool(10, 100);
    for (int i=0;i<100;i++) {
        int* n = new int(i + 100);
        pool.addTask(Task<int>(taskFunc, n));
    }
    
    sleep(30);

    printf("======= END =======\n");
    return 0;
}