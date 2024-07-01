#include <iostream>
#include <string>
#include <thread>
#include <mutex>

/*

void foo(int& x) {
    x++;
    std::cout << "a = " << x << std::endl;
}

int main() {
    int a = 1;
    std::thread t1 (foo, std::ref(a));   // ref将a转化为引用类型
    t1.join();      
    std::cout << "function main" << std::endl;

    return 0;
}

*/

/* 单例模式: 某个类只能创建一个实例且全局唯一  ==> 需注意线程安全 */
class Log {
public:
    // 禁止拷贝构造函数和赋值运算符
    Log(const Log& log) = delete;
    Log& operator=(const Log& log) = delete;

    // 获取实例的静态方法
    static Log& GetInstance() {
        std::call_once(onceFlag, &Log::init);
        return *m_log;
    }

    void printLog(const std::string& msg) {
        std::cout <<  __TIME__ << ' ' << msg << std::endl;
    }

private:
    // 私有构造函数
    Log() {}

    // 初始化函数，用于创建单例实例
    static void init() {
        if (!m_log) m_log = new Log();
    }

    static Log* m_log;         // 静态指针，指向单例实例
    static std::once_flag onceFlag;  // 用于保证初始化只执行一次
};

// 静态成员变量定义 
Log* Log::m_log = nullptr;
std::once_flag Log::onceFlag;

void print_error() {
    Log::GetInstance().printLog("error");
}

int main() {
    std::thread t1(print_error);
    std::thread t2(print_error);
    t1.join();
    t2.join();
    return 0;
}