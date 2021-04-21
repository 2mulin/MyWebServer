/***********************************************************
 *@author RedDragon
 *@date 2021/3/23
 *@brief 线程池
 * 任务队列使用new出来的数组实现, 轮流选取
 * 使用消费者生产者模型
 * 最开始线程数为0, 随着任务的添加,开始创建线程,
 * 当线程数目大于maxThreadCount时,线程不再创建
 * 当空闲时, 逐渐删除线程, 只留下核心线程
***********************************************************/

#ifndef WEBSERVER_THREADPOOL_H
#define WEBSERVER_THREADPOOL_H
#include <pthread.h>
#include <vector>
#include <functional>
using std::function;
using std::vector;

enum class ThreadPoolStatus{
    LOCK_FAILURE,
    QUEUE_FULL,
    SHUTDOWN
};

struct Task
{
    function<void(void*)> func;
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

#endif //WEBSERVER_THREADPOOL_H
