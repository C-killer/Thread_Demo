#include "threadPool.hpp"
#include <iostream>
#include <cstring>
#include <string>
#include <unistd.h>

#define NUMBER 2

template <typename T>
ThreadPool<T>::ThreadPool(int min, int max) : m_minNum(min), m_maxNum(max), m_aliveNum(min)
{
    do {
        // 实例化任务队列
        m_taskQ = new TaskQueue<T>;
        if (m_taskQ == nullptr) {
            std::cout << "Failed to allocate memory for taskQ." << std::endl;
            break;
        }
        m_threadIDs = new pthread_t[m_maxNum];
        if (m_threadIDs == nullptr) {
            std::cout << "Failed to allocate memory for threadIDs." << std::endl;
            break;
        }
        memset(m_threadIDs, 0, sizeof(std::thread) * m_maxNum);

        // 初始化互斥锁,条件变量
        if (pthread_mutex_init(&m_lock, NULL) != 0 ||
            pthread_cond_init(&m_notEmpty, NULL) != 0)
        {
            std::cout << "init mutex or condition fail..." << std::endl;
            break;
        }

        // 创建线程
        pthread_create(&m_managerID, NULL, manager, this);
        for (int i = 0; i < m_minNum; ++i)
        {
            pthread_create(&m_threadIDs[i], NULL, worker, this);
            std::cout << "create thread ID: " << std::to_string(m_threadIDs[i]) << std::endl;
        }
        return ;    // 创建成功后阻止继续向下执行   
    } while (0);

    // 释放资源
    if (m_threadIDs) delete[] m_threadIDs;
    if (m_taskQ) delete m_taskQ;
}


template <typename T>
ThreadPool<T>::~ThreadPool(){
    m_shutdown = true;
    // 销毁管理者线程
    pthread_join(m_managerID, NULL);
    // 唤醒所有消费者线程
    for (int i = 0; i < m_aliveNum; ++i){
        pthread_cond_signal(&m_notEmpty);
    }
    if (m_taskQ) delete m_taskQ;
    if (m_threadIDs) delete[]m_threadIDs;
    pthread_mutex_destroy(&m_lock);
    pthread_cond_destroy(&m_notEmpty);
}

template <typename T>
void* ThreadPool<T>::worker(void* arg) {
    ThreadPool* pool = static_cast<ThreadPool*>(arg);   // 强制类型转换
    // 线程进入函数后需要不断读取任务队列，因此使用死循环
    while (true) {
        // 访问任务队列(共享资源)加锁
        pthread_mutex_lock(&pool->m_lock);
        // 判断任务队列是否为空, 如果为空工作线程阻塞
        while (pool->m_taskQ->taskNumber() == 0 && !pool->m_shutdown)
        {
            std::cout << "thread " << std::to_string(pthread_self()) << " waiting..." << std::endl;
            // 阻塞线程
            pthread_cond_wait(&pool->m_notEmpty, &pool->m_lock);

            // 解除阻塞之后, 判断是否要销毁线程
            if (pool->m_exitNum > 0)
            {
                pool->m_exitNum--;
                if (pool->m_aliveNum > pool->m_minNum)
                {
                    pool->m_aliveNum--;
                    pthread_mutex_unlock(&pool->m_lock);
                    pool->threadExit();
                }
            }
        }
        // 判断线程池是否被关闭
        if (pool->m_shutdown) {
            // 解锁避免死锁，然后退出
            pthread_mutex_unlock(&pool->m_lock);
            pool->threadExit();
        }

        // 从任务队列中取出一个任务
        Task task = pool->m_taskQ->takeTask();
        // 工作的线程+1
        pool->m_busyNum++;
        // 线程池解锁
        pthread_mutex_unlock(&pool->m_lock);
        // 执行任务
        std::cout << "thread " << std::to_string(pthread_self()) << " start working..." << std::endl;
        task.function(task.arg);
        delete task.arg;
        task.arg = nullptr;

        // 任务处理结束
        std::cout << "thread " << std::to_string(pthread_self()) << " end working..." << std::endl;
        pthread_mutex_lock(&pool->m_lock);
        pool->m_busyNum--;
        pthread_mutex_unlock(&pool->m_lock);
    }
    return nullptr;
}

template <typename T>
void* ThreadPool<T>::manager(void* arg) {
    ThreadPool* pool = static_cast<ThreadPool*>(arg);   // 强制类型转换
    while (!pool->m_shutdown) {
        // 每隔三秒检测一次
        sleep(3);

        // 读取线程池中任务的数量和当前存活线程的数量，由于是共享资源需要加锁
        pthread_mutex_lock(&pool->m_lock);
        int queueSize = pool->m_taskQ->taskNumber();
        int liveNum = pool->m_aliveNum;
        int busyNum = pool->m_busyNum;
        pthread_mutex_unlock(&pool->m_lock);


        // 添加线程
        // 自定义：任务的个数 > 存活的线程个数 && 存活的线程数 < 最大线程数
        if (queueSize > liveNum && liveNum < pool->m_maxNum) {
            // 线程池加锁
            pthread_mutex_lock(&pool->m_lock);
            int num = 0;
            for (int i = 0; i < pool->m_maxNum && num < NUMBER
                && pool->m_aliveNum < pool->m_maxNum; ++i){
                if (pool->m_threadIDs[i] == 0){
                    pthread_create(&pool->m_threadIDs[i], NULL, worker, pool);
                    num++;
                    pool->m_aliveNum++;
                }
            }
            pthread_mutex_unlock(&pool->m_lock);
        }
        // 销毁线程
        // 自定义：忙的线程*2 < 存活的线程数 && 存活的线程>最小线程数
        if (busyNum * 2 < liveNum && liveNum > pool->m_minNum) {
            pthread_mutex_lock(&pool->m_lock);
            pool->m_exitNum += NUMBER;
            pthread_mutex_unlock(&pool->m_lock);
            // 让工作线程自杀: 在manager中唤醒worker中的线程执行自杀操作
            for (int i = 0; i < NUMBER; i++) {
                pthread_cond_signal(&pool->m_notEmpty);
            }
        }
    }
    return nullptr;
}

template <typename T>
void ThreadPool<T>::threadExit(){
    pthread_t tid = pthread_self();
    for (int i = 0; i < m_maxNum; ++i){
        if (m_threadIDs[i] == tid){
            std::cout << "threadExit() function: thread " 
                << std::to_string(pthread_self()) << " exiting..." << std::endl;
            m_threadIDs[i] = 0;
            break;
        }
    }
    pthread_exit(NULL);
}

template <typename T>
void ThreadPool<T>::addTask(Task<T> task)
{
    if (m_shutdown){ return;}
    // 添加任务，不需要加锁，任务队列中有锁
    m_taskQ->addTask(task);
    // 唤醒工作的线程
    pthread_cond_signal(&m_notEmpty);
}

template <typename T>
int ThreadPool<T>::getBusyNum(){
    int busyNum = 0;
    pthread_mutex_lock(&m_lock);
    busyNum = m_busyNum;
    pthread_mutex_unlock(&m_lock);
    return busyNum;
}

template <typename T>
int ThreadPool<T>::getAliveNum(){
    int threadNum = 0;
    pthread_mutex_lock(&m_lock);
    threadNum = m_aliveNum;
    pthread_mutex_unlock(&m_lock);
    return threadNum;
}

