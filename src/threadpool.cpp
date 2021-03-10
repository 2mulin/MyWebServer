/************************************************************************************
 *@author RedDragon
 *@date 2020/9/5
 *@brief 线程池实现
 * 就是通过pthread_create()创建固定线程数，省去了来一个任务才创建线程和销毁线程
 * 的开销，我这里创建了四个固定线程，通过互斥量mutex和条件变量cond和一个任务队列task_queue添加任务
 * 和完成任务.
 *
 * addTask是生产者
 * worker是消费者
*************************************************************************************/
#include "threadpool.h"
#include <stdexcept>
#include <cstring>

// 唯一单例
threadPool threadPool::pool;

// 消费者线程
void* threadPool::worker(void *arg)
{
    if(arg != nullptr)
        arg = nullptr;
    for(;;)
    {
        pthread_mutex_lock(&pool.mtx);
        while (pool.taskCount <= 0 && !pool.shutdown)
        {// 没有任务就阻塞, 等待任务
            pthread_cond_wait(&pool.cond, &pool.mtx);
        }
        if(pool.shutdown == immediate_shutdown)
            break;
        if(pool.shutdown == graceful_shutdown && pool.taskCount == 0)
            break;// 优雅关闭, 任务做完才关闭

        struct threadPool_task t1 = pool.taskQueue[pool.begin];
        pool.begin = (pool.begin + 1) % pool.queueSize;
        --pool.taskCount;

        pthread_mutex_unlock(&pool.mtx);
        // 执行真正的任务
        (*t1.function)(t1.argument);
    }
    --pool.threadCount;
    pthread_mutex_unlock(&pool.mtx);
    pthread_exit(nullptr);// 线程退出
}

// 公有成员函数
threadPool::threadPool()
    : threadPool(THREAD_COUNT , QUEUE_SIZE)
{}

threadPool::threadPool(int count, int queue_size)
{
    pthread_mutex_init(&mtx, nullptr);
    pthread_cond_init(&cond, nullptr);
    tidArr = new pthread_t[count];
    queueSize = queue_size;
    taskQueue = new threadPool_task[queue_size];
    threadCount = 0;
    for(int i = 0; i < count; ++i){
        int ret = pthread_create(&tidArr[i], nullptr, worker, nullptr);
        if(ret != 0)
        {// 内存等资源不够, 可能创建线程失败
            printf("errno=%d: %s\n",ret, strerror(ret));
            pthread_mutex_destroy(&mtx);
            pthread_cond_destroy(&cond);
            delete tidArr;
            delete taskQueue;
            // 创建线程失败, 提前终止
            throw std::runtime_error("1. 检查内存是否足够, 2. 检查参数是否正确, 如attr是否初始化\n");
        }
        else
            threadCount++;
    }
    begin = end = taskCount = 0;
    shutdown = false;
}
threadPool::~threadPool()
{
    // 如果线程还在运行, 就join
    if(!shutdown)
        joinAll(THREADPOOL_GRACEFUL);
    pthread_mutex_destroy(&mtx);
    pthread_cond_destroy(&cond);
    delete tidArr;
    delete taskQueue;
}

int threadPool::addTask(struct threadPool_task tk)
{
    if(tk.function == nullptr)
        return THREADPOOL_INVALID;
    int ret = pthread_mutex_lock(&mtx); // 加锁
    if(ret != 0)
    {
        printf("%s\n", strerror(ret));
        return THREADPOOL_LOCK_FAILURE;
    }

    int next = (end + 1)%queueSize;
    int err = 0;
    do{
        if(taskCount == queueSize){
            err = THREADPOOL_QUEUE_FULL;
            break;
        }
        if(shutdown){
            err = THREADPOOL_SHUTDOWN;
            break;
        }
        taskQueue[end] = tk;
        ++taskCount;
        end = next;
        ret = pthread_cond_signal(&cond);
        if(ret != 0)
            err = THREADPOOL_LOCK_FAILURE;
    }while(false);

    ret = pthread_mutex_unlock(&mtx);// 解锁
    if(ret != 0)
    {
        printf("%s\n", strerror(ret));
        return THREADPOOL_LOCK_FAILURE;
    }
    return err;
}

int threadPool::joinAll(int flags)
{
    int ret = pthread_mutex_lock(&mtx);
    if(ret != 0)
        return THREADPOOL_LOCK_FAILURE;
    int err = 0;
    do{
        if(shutdown){
            err = THREADPOOL_SHUTDOWN;
            break;
        }
        // 根据flag选择关闭的方式
        shutdown = flags & THREADPOOL_GRACEFUL ? graceful_shutdown : immediate_shutdown;
        // 唤醒所有线程, 所有线程都会退出, 因为shutdown被改变了, 不会再满足while了
        ret = pthread_cond_broadcast(&cond);
        if(ret != 0)
            err = THREADPOOL_LOCK_FAILURE;
        ret = pthread_mutex_unlock(&mtx);
        if(ret != 0)
            err = THREADPOOL_LOCK_FAILURE;

        for(int i = 0; i < threadCount; ++i)
        {
            ret = pthread_join(tidArr[i], nullptr);
            if(ret != 0)
                err = THREADPOOL_JOIN_FAILURE;
            --threadCount;
        }
    }while(false);

    return err;
}