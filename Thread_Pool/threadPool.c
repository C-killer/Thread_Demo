#include "threadPool.h"

#define NUM 2    // 一次添加的线程数

// 任务结构体
typedef struct _Task {
    void (*function)(void* arg);
    void* arg;
} Task;

// 线程池结构体
typedef struct _threadPool {
    Task* taskQ;               // 任务队列
    int queueCapacity;         // 队列容量
    int queueSize;             // 当前任务数
    int queueFront;            // 队列头索引
    int queueRear;             // 队列尾索引

    pthread_t managerID;       // 管理者线程ID
    pthread_t* threadIDs;      // 线程ID数组
    int minNum;                // 最小线程数
    int maxNum;                // 最大线程数
    int busyNum;               // 忙线程数
    int liveNum;               // 存活线程数
    int exitNum;               // 要退出的线程数

    pthread_mutex_t mutexPool; // 线程池互斥锁
    pthread_mutex_t mutexBusy; // 忙线程数互斥锁
    pthread_cond_t notFull;    // 队列不满条件变量
    pthread_cond_t notEmpty;   // 队列不空条件变量

    int shutDown;              // 线程池关闭标志
} ThreadPool;

void threadExit(ThreadPool* pool);

ThreadPool* createThreadPool(int queueCapacity, int minNum, int maxNum) {
    ThreadPool* pool = (ThreadPool*)malloc(sizeof(ThreadPool));
    // 使用do...while...搭配break的方式，有效避免因malloc fail异常退出导致的内存泄漏问题
    do {
        if (!pool) {
            printf("Failed to allocate memory for thread pool.\n");
            break;
        }

        pool->threadIDs = (pthread_t*)malloc(sizeof(pthread_t) * maxNum);
        if (!pool->threadIDs) {
            printf("Failed to allocate memory for thread IDs.\n");
            break;
        }
        // 初始化为0，后期可以根据0判断此数组中是否被占用。 pthread_t本质上是无符号长整形
        memset(pool->threadIDs, 0, sizeof(pthread_t) * maxNum);

        // 任务队列
        pool->taskQ = (Task*)malloc(sizeof(Task) * queueCapacity);
        if (!pool->taskQ) {
            printf("Failed to allocate memory for task queue.\n");
            break;
        }
        pool->queueCapacity = queueCapacity;
        pool->queueSize = 0;
        pool->queueFront = 0;
        pool->queueRear = 0;

        pool->minNum = minNum;
        pool->maxNum = maxNum;
        pool->busyNum = 0;
        pool->liveNum = minNum;
        pool->exitNum = 0;
        pool->shutDown = 0;

        if (pthread_mutex_init(&pool->mutexPool, NULL) != 0 ||
            pthread_mutex_init(&pool->mutexBusy, NULL) != 0 ||
            pthread_cond_init(&pool->notFull, NULL) != 0 ||
            pthread_cond_init(&pool->notEmpty, NULL) != 0) {
            printf("Failed to initialize mutex or condition variables.\n");
            break;
        }

        // 创建线程
        pthread_create(&pool->managerID, NULL, manager, pool);
        for (int i = 0; i < minNum; i++) {
            pthread_create(&pool->threadIDs[i], NULL, worker, pool);
        }

        // 若一切正常则返回pool指针
        return pool;
    } while (0);

    // 若出现问题触发break，则进行内存释放
    if (pool && pool->threadIDs) free(pool->threadIDs);
    if (pool && pool->taskQ) free(pool->taskQ);
    if (pool) free(pool);

    return NULL;
}


void* worker(void* arg) {
    ThreadPool* pool = (ThreadPool*)arg;
    // 线程进入函数后需要不断读取任务队列，因此使用死循环
    while (1) {
        pthread_mutex_lock(&pool->mutexPool);
        // 检查任务队列是否为空
        while (pool->queueSize == 0 && !pool->shutDown) {    // 若当前任务数量为0且线程池没有被关闭则阻塞
            // 阻塞工作线程
            pthread_cond_wait(&pool->notEmpty, &pool->mutexPool);
            // 判断是否要销毁线程
            if (pool->exitNum > 0) {
                pool->exitNum--;
                if (pool->liveNum > pool->minNum) {
                    pool->liveNum--;
                    pthread_mutex_unlock(&pool->mutexPool);   // 在wait自动加锁后解锁
                    threadExit(pool);
                }
            }
        }

        // 判断线程池是否被关闭
        if (pool->shutDown) {
            // 解锁避免死锁，然后退出
            pthread_mutex_unlock(&pool->mutexPool);
            threadExit(pool);
        }

        // 从任务队列中取出一个任务
        Task task;
        memset(&task, 0, sizeof(Task));
        task.function = pool->taskQ[pool->queueFront].function;
        task.arg = pool->taskQ[pool->queueFront].arg;
        // 移动头节点，将数组变为循环队列
        pool->queueFront = (pool->queueFront + 1) % pool->queueCapacity;
        pool->queueSize--;
        // 取出任务后，唤醒“生产者”加入任务
        pthread_cond_signal(&pool->notFull);
        // 解锁
        pthread_mutex_unlock(&pool->mutexPool);

        // 执行任务
        printf("Thread %ld start working...\n", pthread_self());
        pthread_mutex_lock(&pool->mutexBusy);
        pool->busyNum++;
        pthread_mutex_unlock(&pool->mutexBusy);
        task.function(task.arg);
        free(task.arg);

        printf("Thread %ld end working...\n", pthread_self());
        pthread_mutex_lock(&pool->mutexBusy);
        pool->busyNum--;
        pthread_mutex_unlock(&pool->mutexBusy);
    }
    return NULL;
}


void* manager(void* arg) {
    ThreadPool* pool = (ThreadPool*)arg;
    while (!pool->shutDown) {
        // 每隔三秒检测一次
        sleep(3);

        // 读取线程池中任务的数量和当前存活线程的数量，由于是共享资源需要加锁
        pthread_mutex_lock(&pool->mutexPool);
        int queueSize = pool->queueSize;   // 任务的数量
        int liveNum = pool->liveNum;       // 存活线程数量
        pthread_mutex_unlock(&pool->mutexPool);
        // 取出忙的线程的数量
        pthread_mutex_lock(&pool->mutexBusy);
        int busyNum = pool->busyNum;
        pthread_mutex_unlock(&pool->mutexBusy);

        // 添加线程
        // 自定义：任务的个数 > 存活的线程个数 && 存活的线程数 < 最大线程数
        if (queueSize > liveNum && liveNum < pool->maxNum) {
            int count = 0;
            for (int i = 0; i < pool->maxNum && count < NUM && liveNum < pool->maxNum; i++) {
                pthread_mutex_lock(&pool->mutexPool);
                if (pool->threadIDs[i] == 0) {
                    pthread_create(&pool->threadIDs[i], NULL, worker, pool);
                    count++;
                    pool->liveNum++;    // 操作线程池资源需加锁
                }
                pthread_mutex_unlock(&pool->mutexPool);
            }
        }
        // 销毁线程
        // 自定义：忙的线程*2 < 存活的线程数 && 存活的线程>最小线程数
        if (pool->busyNum * 2 < liveNum && liveNum > pool->minNum) {
            pthread_mutex_lock(&pool->mutexPool);
            pool->exitNum += NUM;
            pthread_mutex_unlock(&pool->mutexPool);
            // 让工作线程自杀: 在manager中唤醒worker中的线程执行自杀操作
            for (int i = 0; i < NUM; i++) {
                pthread_cond_signal(&pool->notEmpty);
            }
        }
    }
    return NULL;
}


void threadExit(ThreadPool* pool) {
    pthread_t tid = pthread_self();
    for (int i = 0; i < pool->maxNum; i++) {
        if (pool->threadIDs[i] == tid) {
            pool->threadIDs[i] = 0;
            printf("Thread %ld exiting...\n", tid);
            break;
        }
    }
    pthread_exit(NULL);
}


void threadPoolAdd(ThreadPool* pool, void(*function)(void*), void* arg) {
    pthread_mutex_lock(&pool->mutexPool);

    while (pool->queueSize == pool->queueCapacity && !pool->shutDown) {     // 若任务已满且线程池没有被关闭则阻塞
        // 阻塞生产者线程
        pthread_cond_wait(&pool->notFull, &pool->mutexPool);
    }
    // 若线程池被关闭
    if (pool->shutDown) {
        pthread_mutex_unlock(&pool->mutexPool);
        return;
    }

    // 添加任务
    Task* task = &pool->taskQ[pool->queueRear];
    task->function = function;
    task->arg = arg;
    // 移动队列尾部
    pool->queueRear = (pool->queueRear + 1) % pool->queueCapacity;
    pool->queueSize++;

    pthread_cond_signal(&pool->notEmpty);   // 唤醒worker中等待的线程：唤醒“消费者”从任务队列中取出任务

    pthread_mutex_unlock(&pool->mutexPool);
}


int threadPoolBusyNum(ThreadPool* pool) {
    pthread_mutex_lock(&pool->mutexBusy);
    int busyNum = pool->busyNum;
    pthread_mutex_unlock(&pool->mutexBusy);
    return busyNum;
}


int threadPoolLiveNum(ThreadPool* pool) {
    pthread_mutex_lock(&pool->mutexPool);
    int liveNum = pool->liveNum;
    pthread_mutex_unlock(&pool->mutexPool);
    return liveNum;
}


int threadPoolDestroy(ThreadPool* pool) {
    if (!pool) {
        return -1;
    }
    // 关闭线程池
    pool->shutDown = 1;
    // 阻塞回收管理者线程
    pthread_join(pool->managerID, NULL);
    // 唤醒阻塞的消费者线程
    for (int i = 0; i < pool->liveNum; i++) {
        pthread_cond_signal(&pool->notEmpty);
    }

    if (pool->threadIDs) free(pool->threadIDs);
    if (pool->taskQ) free(pool->taskQ);

    pthread_mutex_destroy(&pool->mutexPool);
    pthread_mutex_destroy(&pool->mutexBusy);
    pthread_cond_destroy(&pool->notEmpty);
    pthread_cond_destroy(&pool->notFull);

    free(pool);

    return 0;
}
