#include "TaskQueue.hpp"

template <typename T>
TaskQueue<T>::TaskQueue() {
    pthread_mutex_init(&m_mutex, NULL);
}

template <typename T>
TaskQueue<T>::~TaskQueue() {
    pthread_mutex_destroy(&m_mutex);
}

template <typename T>
void TaskQueue<T>::addTask(Task<T> task) {
    pthread_mutex_lock(&m_mutex);
    m_taskQ.push(task);
    pthread_mutex_unlock(&m_mutex);
}

template <typename T>
void TaskQueue<T>::addTask(callback f, void* arg) {
    pthread_mutex_lock(&m_mutex);
    Task<T> task;
    task.function = f;
    task.arg = arg;
    m_taskQ.push(task);
    pthread_mutex_unlock(&m_mutex);
}

template <typename T>
Task<T> TaskQueue<T>::takeTask() {
    Task<T> task;
    pthread_mutex_lock(&m_mutex);
    if (!m_taskQ.empty()) {
        task = m_taskQ.front();
        m_taskQ.pop();
    }
    pthread_mutex_unlock(&m_mutex);
    return task;
}

