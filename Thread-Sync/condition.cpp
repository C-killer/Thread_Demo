/* 条件变量与互斥锁搭配实现线程同步
 * 使用条件变量处理生产-消费者模型
 * 10个线程，5个线程为生产者，向链表头部添加节点；5个线程为消费者，从链表头部删除一个节点
 */
#include <iostream>
#include <pthread.h>
#include <unistd.h>

pthread_cond_t cond;
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
        pthread_mutex_lock(&mutex);
        struct Node* newNode = (struct Node*)(malloc(sizeof(struct Node)));
        newNode->num = rand()%100;
        newNode->next = head;
        head = newNode;
        std::cout << "Producer id : " << pthread_self() << " Node : num = " << newNode->num << std::endl;
        pthread_mutex_unlock(&mutex);
        pthread_cond_signal(&cond);  // 唤醒被阻塞的消费者进程
        sleep(rand()%3);
    }
    return NULL;
}


void* consumer(void* arg) {
    while (1) {
        pthread_mutex_lock(&mutex);
        while (head == NULL) {
            pthread_cond_wait(&cond,&mutex); //阻塞消费者进程
        }
        struct Node* node = head;
        std::cout << "Consumer id : " << pthread_self() << " Node : num = " << node->num << std::endl;
        head = head->next;
        free(node);
        pthread_mutex_unlock(&mutex);

        sleep(rand()%3);
    }
    return NULL;
}


int main() {
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);

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
    pthread_cond_destroy(&cond);
}


/*
为什么在第40行使用 if 有bug:
    当任务队列为空, 所有的消费者线程都会被这个函数阻塞 pthread_cond_wait(&cond, &mutex);
    也就是阻塞在代码的第9行
	
    当生产者生产了1个节点, 调用 pthread_cond_broadcast(&cond); 唤醒了所有阻塞的线程
      - 有一个消费者线程通过 pthread_cond_wait()加锁成功, 其余没有加锁成功的线程继续阻塞
      - 加锁成功的线程向下运行, 并成功删除一个节点, 然后解锁
      - 没有加锁成功的线程解除阻塞继续抢这把锁, 另外一个子线程加锁成功
      - 但是这个线程删除链表节点的时候链表已经为空了, 后边访问这个空节点的时候就会出现段错误
    解决方案:
      - 需要循环的对链表是否为空进行判断, 需要将if 该成 while
*/