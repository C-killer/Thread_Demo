/* 资源总数为1时，仅用信号量实现线程同步
 * 资源总素大于1时，信号量和互斥锁实现线程同步
 * 使用信号量实现生产-消费者模型
 * 10个线程，5个线程为生产者，向链表头部添加节点；5个线程为消费者，从链表头部删除一个节点
 */
 
#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

sem_t semp;    // 生产者信号量
sem_t semc;    // 消费者信号量
pthread_mutex_t mutex;

//链表节点类型
struct Node {
    int num;
    struct Node* next;
};
//创建头节点    
struct Node* head = NULL;


void* producer(void* arg) {
    while (1) {
        sem_wait(&semp);
        pthread_mutex_lock(&mutex);  //放在sem里，防止带着锁一起被阻塞
        struct Node* newNode = (struct Node*)(malloc(sizeof(struct Node)));
        newNode->num = rand()%100;
        newNode->next = head;
        head = newNode;
        std::cout << "Producer id : " << pthread_self() << " Node : num = " << newNode->num << std::endl;
        pthread_mutex_unlock(&mutex);
        sem_post(&semc);  // 通知消费者生产完成可以消费,给消费者资源+1
        sleep(rand()%3);
    }
    return NULL;
}


void* consumer(void* arg) {
    while (1) {
        sem_wait(&semc);
        pthread_mutex_lock(&mutex);
        struct Node* node = head;
        std::cout << "Consumer id : " << pthread_self() << " Node : num = " << node->num << std::endl;
        head = head->next;
        free(node);
        pthread_mutex_unlock(&mutex);
        sem_post(&semp);   // 通知生产者消费完成可以生产,给生产者资源+1
        sleep(rand()%3);
    }
    return NULL;
}


int main() {
    /* 资源总数为1，相当于只有一个乒乓球五个人抢着把球打出去，五个人抢着把球打回来 
     * 此时，不需要互斥锁进行线程同步
    sem_init(&semp, 0, 1);  // 生产者
    sem_init(&semc, 0, 0);  // 消费者 -> 资源数量初始化为0，消费者线程启动后自动阻塞    
    */


    sem_init(&semp, 0, 5);  // 生产者 -> 资源数量大于1，使用互斥锁避免多个线程在同一时间对同一地址进行更改
    sem_init(&semc, 0, 0);  // 消费者 -> 资源数量初始化为0，消费者线程启动后自动阻塞    
    pthread_mutex_init(&mutex, NULL);

    pthread_t p_produ[5], p_consu[5];
    for (int i=0;i<5;i++) {
        pthread_create(&p_produ[i], NULL, producer, NULL);
    }
    for (int i=0;i<5;i++) {
        pthread_create(&p_consu[i], NULL, consumer, NULL);
    }

    for (int i=0;i<5;i++) {
        pthread_join(p_produ[i],NULL);
        pthread_join(p_consu[i],NULL);
    }

    pthread_mutex_destroy(&mutex);
    sem_destroy(&semp);
    sem_destroy(&semc);
}

