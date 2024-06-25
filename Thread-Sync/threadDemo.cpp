#include <iostream>
#include <pthread.h>

struct Person
{
    int id;
    int age;
};


void* callBack(void* arg) {
    for (int i=0;i<5;i++) {
        std::cout << "子线程1 : " << i << std::endl;
    }
    std::cout << "子线程1 : " << pthread_self() << std::endl;    // 返回线程id

    struct Person *pp = (struct Person *)(arg);
    std::cout << "子线程1中 pp 的id : " << pp->id << " age : " << pp->age << std::endl;

    static Person p = {1, 20};
    pthread_exit(&p);   // 向主线程传递消息p

    return NULL;
}


void* working(void* arg) {
    std::cout << "子线程2 id: " << pthread_self() << std::endl;
    for (int i=0;i<5;i++) {
        std::cout << "= = = = = =" << std::endl;
    }
    return NULL;
}


int main() {
    struct Person pp = {2, 10};
    pthread_t pid1,pid2;    //本质是无符号长整型
    std::cout << "主线程 id: " << pthread_self() << std::endl;    // 返回线程id
    pthread_create(&pid1, NULL, callBack, &pp);  // pp的地址被传递给callBack
    pthread_create(&pid2, NULL, working, NULL);

    //在此期间父子线程随机交替执行

    /* 使用join阻塞子线程 
    //pthread_join(pid,NULL);   // 不接收子线程数据
    void* ptr;
    pthread_join(pid, &ptr);    //阻塞主线程直到子线程结束；接收子线程数据至ptr；一个join只能阻塞一个子线程
    struct Person *p = (struct Person *)ptr;
    std::cout << "主线程中 p 的id : " << p->id << " age : " << p->age << std::endl;
    */

    //若阻塞过多则主线程无法执行

    /* 用线程分离解决join带来的阻塞问题 */  
    pthread_detach(pid1);    // 将pid1代表线程与主线程分离,此后子线程资源由其他进程回收

    pthread_cancel(pid2);    // 取消线程pid2，pid2在进行一次系统调用后取消
    pthread_join(pid2,NULL);
    std::cout << "主线程退出" << std::endl;
    pthread_exit(NULL);     //使用exit结束主线程不影响子线程地址空间

    return 0;
}