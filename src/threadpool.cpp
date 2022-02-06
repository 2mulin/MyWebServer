#include "threadpool.h"
#include <cstring>

// 消费者线程
void* threadPool::worker(void* arg)
{
    if(arg == nullptr)
        return nullptr;
    threadPool* pool = (threadPool*)arg;
    for(;;)
    {
        int err = pthread_mutex_lock(pool->mtx);
        if(err != 0)
        {
            printf("errno = %d: %s", err, strerror(err));
            break;
        }

        while (pool->taskCount <= 0)
            pthread_cond_wait(pool->cond, pool->mtx);
        // 拿到任务
        Task tk = pool->taskQueue[pool->begin];
        pool->begin += 1;
        pool->taskCount -= 1;
        err = pthread_mutex_unlock(pool->mtx);
        if(err != 0)
        {
            printf("errno = %d: %s", err, strerror(err));
            break;
        }
        // 执行任务
        tk.func(tk.arg);
    }
    return nullptr;
}

/*
 * @brief: 创建count个工作线程
 * @return 成功创建的线程数
 */
int threadPool::createThread(int count)
{
    int old = threadCount;
    for(int i = 0; i < count; ++i)
    {
        pthread_t newTid;
        int ret = pthread_create(&newTid, nullptr, worker, this);
        if(ret != 0)
        {// 内存等资源不够, 可能创建线程失败
            printf("errno=%d: %s\n",ret, strerror(ret));
            break;
        }
        else
        {
            printf("创建线程成功!\n");
            tidVec.push_back(newTid);
            threadCount++;
        }
    }
    return threadCount - old;
}

// 构造函数(线程数初始为4)
threadPool::threadPool()
    : threadPool(0)
{}

threadPool::threadPool(int count)
{
    mtx = new pthread_mutex_t;
    cond = new pthread_cond_t;
    pthread_mutex_init(mtx, nullptr);
    pthread_cond_init(cond, nullptr);

    coreThreadCount = count;
    threadCount = 0;

    taskQueue = new Task[QUEUE_SIZE];

    begin = end = taskCount = 0;
    shutdown = false;
}

threadPool::~threadPool()
{
    // 如果线程还在运行, 就join
//    if(!shutdown)
//        joinAll(THREADPOOL_GRACEFUL);
    pthread_mutex_destroy(mtx);
    pthread_cond_destroy(cond);
    delete mtx;
    delete cond;
    delete[] taskQueue;
}

ThreadPoolStatus threadPool::addTask(Task tk)
{
    int ret = pthread_mutex_lock(mtx); // 加锁
    if(ret != 0)
    {
        printf("%s\n", strerror(ret));
        return ThreadPoolStatus::LOCK_FAILURE;
    }

    int next = (end + 1)%QUEUE_SIZE;
    ThreadPoolStatus err;
    do{
        if(taskCount == QUEUE_SIZE){
            err = ThreadPoolStatus::QUEUE_FULL;
            break;
        }
        if(shutdown){
            err = ThreadPoolStatus::SHUTDOWN;
            break;
        }
        // 创建线程
        if(threadCount < maxThreadCount)
            createThread(1);
        // 增加任务
        taskQueue[end] = tk;
        ++taskCount;
        end = next;
        // 通知工作线程
        ret = pthread_cond_signal(cond);
        if(ret != 0)
            err = ThreadPoolStatus::LOCK_FAILURE;
    }while(false);

    ret = pthread_mutex_unlock(mtx);// 解锁
    if(ret != 0)
    {
        printf("%s\n", strerror(ret));
        return ThreadPoolStatus::LOCK_FAILURE;
    }
    return err;
}

ThreadPoolStatus threadPool::joinAll()
{
    int ret = pthread_mutex_lock(mtx);
    if(ret != 0)
        return ThreadPoolStatus::LOCK_FAILURE;
    ThreadPoolStatus err;
    do{
        if(shutdown){
            err = ThreadPoolStatus::SHUTDOWN;
            break;
        }
        // 唤醒所有线程, 所有线程都会退出
        ret = pthread_cond_broadcast(cond);
        if(ret != 0)
            err = ThreadPoolStatus::LOCK_FAILURE;
        ret = pthread_mutex_unlock(mtx);
        if(ret != 0)
            err = ThreadPoolStatus::LOCK_FAILURE;

        for(int i = 0; i < threadCount; ++i)
        {
            ret = pthread_join(tidVec.back(), nullptr);
            if(ret != 0)
                err = ThreadPoolStatus::LOCK_FAILURE;
            --threadCount;
            tidVec.pop_back();
        }
        shutdown = true;

    }while(false);
    return err;
}