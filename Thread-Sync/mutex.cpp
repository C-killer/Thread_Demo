#include <iostream>
#include <chrono>
#include <thread>
#include <mutex>
using namespace std;

/* 互斥锁的个数和共享资源的个数相等，
 * 也就是说每一个共享资源都应该对应一个互斥锁对象。
 * 互斥锁对象的个数和线程的个数没有关系。
 */
int g_num = 0;  // 为 g_num_mutex 所保护
mutex g_num_mutex;

void slow_increment(int id)
{
    for (int i = 0; i < 3; ++i) 
    {
        /* 使用lock()，unlock()进行加锁解锁 
        g_num_mutex.lock();
        ++g_num;
        cout << id << " => " << g_num << endl;
        g_num_mutex.unlock(); */

        /* 使用哨兵锁lock_guard会自动锁定互斥量，而在退出作用域后进行析构时就会自动解锁，
         * 从而保证了互斥量的正确操作 。
         * 弊端：整个for循环的体都被当做了临界区，临界区越大程序效率越低 */
        lock_guard<mutex> lock(g_num_mutex);
        ++g_num;
        cout << id << " => " << g_num << endl;

        this_thread::sleep_for(chrono::seconds(1));
    }
}

int main()
{
    thread t1(slow_increment, 0);
    thread t2(slow_increment, 1);
    t1.join();
    t2.join();
}
