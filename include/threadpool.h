/***********************************************************
 *@author RedDragon
 *@date 2020/9/5
 *@brief 
***********************************************************/

#ifndef WEBSERVER_THREADPOOL_H
#define WEBSERVER_THREADPOOL_H
#include <exception>
#include "requestData.h"
#include "Lock.h"

const int THREAD_COUNT = 4;     // 固定线程数
const int QUEUE_SIZE = 65535;   // 任务队列大小

const int THREADPOOL_INVALID = -1;      // 任务参数不合法
const int THREADPOOL_LOCK_FAILURE = -2; // pthread_mutex_lock或unlock调用失败
const int THREADPOOL_QUEUE_FULL = -3;   // 任务队列满
const int THREADPOOL_SHUTDOWN = -4;     // 表示线程池已经关闭
const int THREADPOOL_JOIN_FAILURE = -5; // join线程失败

const int THREADPOOL_GRACEFUL = 1;      // 优雅关闭线程池

//const int MAX_THREAD_COUNT = 1024;// 一个线程池的线程数最大限制为1024
//const int MAX_QUEUE = 65535; // 一个任务队列最大任务数量限制

enum threadPool_shutdown_t
{
    immediate_shutdown = 1, // 立即
    graceful_shutdown  = 2  // 优雅关闭
};

struct task
{
    void (*function)(void*);    // 函数指针指向执行任务的函数
    void *argument;             // 传递给函数的参数
};

class threadPool
{
private:
    pthread_mutex_t mtx;    // 互斥量
    pthread_cond_t cond;    // 条件变量，用来通知worker thread
    pthread_t *tidArr;      // 所有线程ID
    task *taskQueue;        // 任务队列
    int threadCount;        // 线程数目
    int queueSize;          // 任务队列大小
    int begin;
    int end;
    int taskCount;          // 挂起任务数
    int shutdown;           // 线程池关闭种类

    static void* worker(void* arg);
    // 私有化构造函数
    threadPool() noexcept(false);
    threadPool(int thread_count, int queue_size) noexcept(false);
    ~threadPool();
    static threadPool pool; // 唯一的实例

public:
    threadPool(const threadPool&) = delete;
    threadPool(threadPool&&) = delete;
    threadPool& operator=(const threadPool&) = delete;

    // 得到实例
    static threadPool& getInstance(){
        return pool;
    };
    // 添加任务
    int addTask(struct task tk);
    // join所有线程
    int joinAll(int flags);
};

#endif //WEBSERVER_THREADPOOL_H
