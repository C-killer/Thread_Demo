/* 读写锁中读操作并行写操作串行，因此在读操作大于写操作的情况下，读写锁的效率高于互斥锁；相反则区别不大
 * 在读写锁中，写的优先级高于读
 *
 * 8个线程操作同一个全局变量，3个线程不定时写，5个线程不定时读  
 */

#include <pthread.h>
#include <unistd.h>
#include <iostream>
#define MAX 50

int num = 0;
pthread_rwlock_t rwlock;

void* read_num(void* arg) {
    for (int i=0;i<MAX;i++) {
        pthread_rwlock_rdlock(&rwlock);     //读操作，因此加读锁
        std::cout << "thread_read id: " << pthread_self() << ", num = " << num << std::endl;
        pthread_rwlock_unlock(&rwlock);
        usleep(rand()%5);
    }
    return NULL;
}

void* write_num(void* arg) {
    for (int i=0;i<MAX;i++) {
        pthread_rwlock_wrlock(&rwlock);     //写操作，因此加写锁
        int cur = num;
        cur++;
        num = cur;
        std::cout << "thread_write id: " << pthread_self() << ", num = " << num << std::endl;
        pthread_rwlock_unlock(&rwlock);
        usleep(5);
    }
    return NULL;
}

int main() {
    pthread_t p_w[3];
    pthread_t p_r[5];
    pthread_rwlock_init(&rwlock, NULL);   // NULL默认属性

    //创建线程
    for (int i=0;i<5;i++) {
        pthread_create(&p_r[i],NULL,read_num,NULL);
    }
    for (int i=0;i<3;i++) {
        pthread_create(&p_w[i],NULL,write_num,NULL);

    }
    //回收线程
    for (int i=0;i<5;i++) {
        pthread_join(p_r[i],NULL);
    }
    for (int i=0;i<3;i++) {
        pthread_join(p_w[i],NULL);
    }

    pthread_rwlock_destroy(&rwlock);
    return 0;
}
 