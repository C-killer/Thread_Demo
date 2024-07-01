#include <iostream>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <functional>
#include <string>
#include <vector>
#include <queue>

class ThreadPool {
    public:
    ThreadPool(int nbThreads) {
        for (int i=0;i<nbThreads;i++) {
            threads.emplace_back([this] {   // emplace_back原地构造，比需要拷贝的push_back更省资源
                while (true) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(mtx);
                        cv.wait(lock, [this] {     // 若任务队列为空或线程池终止，则阻塞. !(tasks.empty() && !stop)
                            return !tasks.empty() || stop ;     
                        });
                        if (stop || tasks.empty()) {
                            return ;
                        }
                        // 取任务
                        task = std::move(tasks.front());   // move: 左值移动到右值
                        tasks.pop();
                    }
                    task();
                }
            });
        }
    }

    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(mtx);
            stop = true;
        }
        cv.notify_all();
        for (auto& t: threads) {    // auto自动判断类型
            t.join();
        }
    }

    // 添加任务
    template<class F, class ... Args>
    void enqueue(F&& f, Args&&... args) {   // &&为万能引用
        {
            std::unique_lock<std::mutex> lock(mtx);     // 加任务前加锁
            // forward：完美转发 ; 添加任务
            tasks.emplace(std::bind(std::forward<F>(f), std::forward<Args>(args)...));
        }
        cv.notify_one();
    }

    private:
    std::vector<std::thread> threads;      // 线程数组
    std::queue<std::function<void()>> tasks;    // 任务队列
    std::mutex mtx;           
    std::condition_variable cv;
    bool stop = false;      //线程池终止

};

int main() {
    ThreadPool pool(4);

    for (int i=0;i<10;i++) {
        pool.enqueue([i] {
            std::cout << "Task: " << i << " begins" << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
            std::cout << "Task: " << i << " ends" << std::endl;
        });
    }
    std::this_thread::sleep_for(std::chrono::seconds(15));  // 等待所有任务完成
    return 0;
}