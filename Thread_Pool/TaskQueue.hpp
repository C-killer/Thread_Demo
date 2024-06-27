#ifndef TASK_QUEUE_HPP
#define TASK_QUEUE_HPP

#include <queue>
#include <mutex>

using callback = void (*) (void* arg);
template <typename T>
// 任务结构体
struct Task {
    Task<T>() {
        function = nullptr;
        arg = nullptr;
    }
    Task<T>(callback f, void* arg) {
        this->function = f;
        this->arg = (T*)(arg);
    }

    callback function;
    T* arg;
};


template <typename T>
class TaskQueue {
public:
    TaskQueue();
    ~TaskQueue();

    //添加任务
    void addTask(Task<T> task);
    void addTask(callback f, void* arg);
    //取出任务
    Task<T> takeTask();
    //获取当前任务个数
    inline size_t taskNumber() {
        return m_taskQ.size();
    }


private:
    pthread_mutex_t m_mutex;    
    std::queue<Task<T>> m_taskQ;

};

#include "TaskQueue.tpp" // 包含实现文件 : 类模板需要

#endif // TASK_QUEUE_HPP