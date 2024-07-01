# Thread
# Thread-Sync 线程同步

## 互斥锁

```cpp
std::mutex
std::lock_guard
std::recursive_mutex
std::timed_mutex
```
```cpp
pthread_mutex_t  mutex;

// 初始化互斥锁
// restrict: 是一个关键字, 用来修饰指针, 只有这个关键字修饰的指针可以访问指向的内存地址, 其他指针是不行的
int pthread_mutex_init(pthread_mutex_t *restrict mutex,
           const pthread_mutexattr_t *restrict attr);

// 释放互斥锁资源            
int pthread_mutex_destroy(pthread_mutex_t *mutex);

// 修改互斥锁的状态, 将其设定为锁定状态, 这个状态被写入到参数 mutex 中
int pthread_mutex_lock(pthread_mutex_t *mutex);

// 尝试加锁
int pthread_mutex_trylock(pthread_mutex_t *mutex);

// 对互斥锁解锁
int pthread_mutex_unlock(pthread_mutex_t *mutex);
```


## 死锁

## 读写锁
```cpp
pthread_rwlock_t rwlock;

#include <pthread.h>
pthread_rwlock_t rwlock;

// 初始化读写锁
int pthread_rwlock_init(pthread_rwlock_t *restrict rwlock,
           const pthread_rwlockattr_t *restrict attr);

// 释放读写锁占用的系统资源
int pthread_rwlock_destroy(pthread_rwlock_t *rwlock);

// 在程序中对读写锁加读锁, 锁定的是读操作
int pthread_rwlock_rdlock(pthread_rwlock_t *rwlock);

// 这个函数可以有效的避免死锁
// 如果加读锁失败, 不会阻塞当前线程, 直接返回错误号
int pthread_rwlock_tryrdlock(pthread_rwlock_t *rwlock);

// 在程序中对读写锁加写锁, 锁定的是写操作
int pthread_rwlock_wrlock(pthread_rwlock_t *rwlock);

// 这个函数可以有效的避免死锁
// 如果加写锁失败, 不会阻塞当前线程, 直接返回错误号
int pthread_rwlock_trywrlock(pthread_rwlock_t *rwlock);

// 解锁, 不管锁定了读还是写都可用解锁
int pthread_rwlock_unlock(pthread_rwlock_t *rwlock);

```

## 条件变量

```cpp
#include <pthread.h>
pthread_cond_t cond;
// 初始化
int pthread_cond_init(pthread_cond_t *restrict cond,
      const pthread_condattr_t *restrict attr);
// 销毁释放资源        
int pthread_cond_destroy(pthread_cond_t *cond);

// 线程阻塞函数, 哪个线程调用这个函数, 哪个线程就会被阻塞
int pthread_cond_wait(pthread_cond_t *restrict cond, pthread_mutex_t *restrict mutex);

// 表示的时间是从1971.1.1到某个时间点的时间, 总长度使用秒/纳秒表示
struct timespec {
	time_t tv_sec;      /* Seconds */
	long   tv_nsec;     /* Nanoseconds [0 .. 999999999] */
};

// 将线程阻塞一定的时间长度, 时间到达之后, 线程就解除阻塞了
int pthread_cond_timedwait(pthread_cond_t *restrict cond,
           pthread_mutex_t *restrict mutex, const struct timespec *restrict abstime);

time_t mytim = time(NULL);	// 1970.1.1 0:0:0 到当前的总秒数
struct timespec tmsp;
tmsp.tv_nsec = 0;
tmsp.tv_sec = time(NULL) + 100;	// 线程阻塞100s

// 唤醒阻塞在条件变量上的线程, 至少有一个被解除阻塞
int pthread_cond_signal(pthread_cond_t *cond);

// 唤醒阻塞在条件变量上的线程, 被阻塞的线程全部解除阻塞
int pthread_cond_broadcast(pthread_cond_t *cond);
```

## 信号量
```cpp
#include <semaphore.h>
sem_t sem;

// 初始化信号量/信号灯
int sem_init(sem_t *sem, int pshared, unsigned int value);

// 资源释放, 线程销毁之后调用这个函数即可
// 参数 sem 就是 sem_init() 的第一个参数            
int sem_destroy(sem_t *sem);
```
### 参数:
#### sem：信号量变量地址
#### pshared： 0：线程同步; 非0：进程同步

```cpp
// 参数 sem 就是 sem_init() 的第一个参数  
// 函数被调用sem中的资源就会被消耗1个, 资源数-1
int sem_wait(sem_t *sem);

// 参数 sem 就是 sem_init() 的第一个参数  
// 函数被调用sem中的资源就会被消耗1个, 资源数-1
int sem_trywait(sem_t *sem);

// 表示的时间是从1971.1.1到某个时间点的时间, 总长度使用秒/纳秒表示
struct timespec {
	time_t tv_sec;      /* Seconds */
	long   tv_nsec;     /* Nanoseconds [0 .. 999999999] */
};

// 调用该函数线程获取sem中的一个资源，当资源数为0时，线程阻塞，在阻塞abs_timeout对应的时长之后，解除阻塞。
// abs_timeout: 阻塞的时间长度, 单位是s, 是从1970.1.1开始计算的
int sem_timedwait(sem_t *sem, const struct timespec *abs_timeout);

// 调用该函数给sem中的资源数+1
int sem_post(sem_t *sem);

// 查看信号量 sem 中的整形数的当前值, 这个值会被写入到sval指针对应的内存中
// sval是一个传出参数
int sem_getvalue(sem_t *sem, int *sval);
```

# Thread Pool线程池
## 原理
线程池的组成主要分为3个部分，这三部分配合工作就可以得到一个完整的线程池：

1. 任务队列，存储需要处理的任务，由工作的线程来处理这些任务
   - 通过线程池提供的API函数，将一个待处理的任务添加到任务队列，或者从任务队列中删除
   - 已处理的任务会被从任务队列中删除
   - 线程池的使用者，也就是调用线程池函数往任务队列添加任务的线程就是生产者线程

2. 工作的线程（任务队列任务的消费者），N个
   - 线程池中维护了一定数量的工作线程，他们的作用是不停的读任务队列，从里边取出任务并处理
   - 工作的线程相当于是任务队列的消费者角色
   - 如果任务队列为空，工作的线程将会被阻塞（使用条件变量/信号量阻塞）
   - 如果阻塞之后有了新的任务，由生产者将阻塞解除，工作线程开始工作

3. 管理者线程（不处理任务队列中的任务），1个
   - 它的任务是周期性的对任务队列中的任务数量以及处于忙忙状态的工作线程个数进行检测
   - 当任务过多的时候，可以适当的创建一些新的工作线程
   - 当任务过少的时候，可以适当的销毁一些工作线程

## C语言实现
### ThreadPool.c
## C++实现
### ThreadPool.cpp (初级版)
### thread_cpp.cpp (高级版）

# 内容参考
    https://subingwen.cn


