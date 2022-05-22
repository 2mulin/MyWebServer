/**
 *@author 2mu
 *@date 2022/5/10
 *@brief 线程池
 */

#ifndef WEBSERVER_THREAD_POOL_H
#define WEBSERVER_THREAD_POOL_H

#include <vector>
#include <functional>
#include <future>

#include <pthread.h>

enum class ThreadPoolStatus{
    LOCK_FAILURE,
    QUEUE_FULL,
    SHUTDOWN
};


/**
 * @brief 好好想想这个Task怎么实现，不能限制函数类型，返回值类型，参数个数，看起来就是模板。
 *
 * 我应该可以使用std::promise和std::future来实现返回值。
 */

template<typename Func, typename args>
struct Task
{
    std::function<void(void*)> func;
    void* arg;
};

class threadPool
{
    const int maxThreadCount = 128; // 最大线程数
    const int QUEUE_SIZE = 20000;   // 任务队列大小
private:
    pthread_mutex_t* mtx;       // 互斥量
    pthread_cond_t* cond;       // 条件变量，用来通知worker thread
    vector<pthread_t> tidVec;   // 所有线程ID
    int threadCount;            // 线程数目
    int coreThreadCount;        // 核心线程数(线程最少保持这个数目)
    Task *taskQueue;            // 任务队列
    int taskCount;              // 任务数
    int begin;
    int end;
    bool shutdown;              // 线程池正在关闭

    static void* worker(void*);
    int createThread(int count);

public:
    threadPool() noexcept(false);
    threadPool(int thread_count);
    ~threadPool();
    threadPool(const threadPool&) = delete;
    threadPool& operator=(const threadPool&) = delete;

    // 添加任务
    ThreadPoolStatus addTask(Task);
    // join所有线程
    ThreadPoolStatus joinAll();
};

#endif //WEBSERVER_THREAD_POOL_H
