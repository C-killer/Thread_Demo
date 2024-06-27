#ifndef THREAD_POOL_HPP
#define THREAD_POOL_HPP

#include <thread>
#include <condition_variable>
#include "TaskQueue.hpp"


template <typename T>
class ThreadPool {
public:
    ThreadPool(int minNum, int maxNum); 
    ~ThreadPool();

    // 添加任务到线程池
    void addTask(Task<T> task);

    // 当前正在工作的线程个数
    int getBusyNum();

    // 当前活着的线程个数
    int getAliveNum();

private:
    static void* worker(void* arg);    // 无需暴露给用户，因此设为私有即可
    static void* manager(void* arg);

    // 单个线程退出
    void threadExit();

    pthread_mutex_t m_lock;
    pthread_cond_t m_notEmpty;
    pthread_t* m_threadIDs;
    pthread_t m_managerID;
    TaskQueue<T>* m_taskQ;
    int m_minNum;
    int m_maxNum;
    int m_busyNum = 0;
    int m_aliveNum;
    int m_exitNum = 0;
    bool m_shutdown = false;
};

#include "ThreadPool.tpp" // 包含实现文件

#endif  // THREAD_POOL_HPP