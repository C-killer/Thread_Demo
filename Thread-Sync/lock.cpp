#include <iostream>
#include <pthread.h>
#include <unistd.h>     //sleep()

#define MAX 50

int num = 0;
pthread_mutex_t mutex;

void* funA_num(void* arg) {
    for (int i=0;i<MAX;i++) {
        pthread_mutex_lock(&mutex);     //加锁保护共享资源在同一时间只被一个线程访问
        /* 尽量增加cpu运算时间 */
        int cur = num;
        cur++;
        usleep(10);
        num = cur;
        std::cout << "thread_A id: " << pthread_self() << ", num = " << num << std::endl;
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}


void* funB_num(void* arg) {
    for (int j=0;j<MAX;j++) {
        pthread_mutex_lock(&mutex);
        /* 尽量增加cpu运算时间 */
        int cur = num;
        cur++;
        usleep(10);
        num = cur;
        std::cout << "thread_B id: " << pthread_self() << ", num = " << num << std::endl;
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}


int main() {
    pthread_t p1, p2;
    pthread_mutex_init(&mutex, NULL);   // NULL默认属性
    pthread_create(&p1, NULL, funA_num, NULL);
    pthread_create(&p2, NULL, funB_num, NULL);

    pthread_join(p1, NULL);
    pthread_join(p2, NULL);
    pthread_mutex_destroy(&mutex);
    return 0;
}