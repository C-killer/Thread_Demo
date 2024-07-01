#include <iostream>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <queue>
#include <string>

std::queue<int> g_queue;
std::condition_variable g_cv;
std::mutex mtx;

void Producer() {
    for (int i=0;i<10;i++) {
        // 此处的{}限定锁的作用域
        {
            std::unique_lock<std::mutex> lock(mtx);
            g_queue.push(i);
            g_cv.notify_one();  // 通知一个线程来取任务 
            std::cout << "Producer: " << i << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }
}

void Consumer() {
    while (1) {
        std::unique_lock<std::mutex> lock(mtx);
        // 若任务队列为空，则等待
        g_cv.wait(lock, [] () { return !g_queue.empty();});
        
        int value = g_queue.front();
        g_queue.pop();
        std::cout << "Consumer: " << value << std::endl;
    }
}

int main() {
    std::thread t1(Producer);
    std::thread t2(Consumer);
    t1.join();
    t2.join();
    return 0;
}

