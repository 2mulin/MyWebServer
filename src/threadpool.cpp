/************************************************************************************
 *@author RedDragon
 *@date 2020/9/5
 *@brief
 *线程池就是通过pthread_create()创建固定线程数，省去了来一个任务才创建线程和销毁线程
 *的开销，我这里创建了四个固定线程，通过互斥量mutex和条件变量cond和一个任务队列task_queue
 *完成一个生产者消费者模型.
 *生产者就是threadpool_add()函数负责添加任务, 由四个线程负责消费（threadpool_thread()完成任务,
 *一旦生产者添加任务，就使用pthread_cond_signal()通知消费者threadpool_thread()去完成任务）
*************************************************************************************/

#include "threadpool.h"

// 被创建的线程（静态函数，只能在本文件中使用）
static void *threadpool_thread(void *threadpool)
{
    threadpool_t *pool = (threadpool_t *)threadpool;

    for(;;)
    {
        pthread_mutex_lock(&(pool->lock));// 加锁

        // 等待条件变量，检查是否有虚假唤醒。

        // 用while循环的原因是pthread_cond_wait返回时，并不能确定判断条件的状态，
        // 所以应该立即重新检查判断条件，确定条件成立（可以消费了）
        // 也可能存在虚假唤醒（错误的唤醒，即使不能生产者没有生产）
        while((pool->count == 0) && (!pool->shutdown))
        {
            pthread_cond_wait(&pool->notify, &pool->lock);
        }

        // 判断线程是否正在关闭，若是再关闭，则该线程将会被关闭
        if((pool->shutdown == immediate_shutdown) ||
           ( (pool->shutdown == graceful_shutdown) && (pool->count == 0) ))
        {
            pthread_mutex_unlock(&(pool->lock));
            break;
        }

        threadpool_task_t task;
        task.function = pool->task_queue[pool->head].function;
        task.argument = pool->task_queue[pool->head].argument;

        // 这就算是消费了，不用task执行完，那太慢了，
        pool->head = (pool->head + 1) % pool->queue_size;
        pool->count -= 1;

        pthread_mutex_unlock(&(pool->lock));// 解锁

        // 执行实际要做的任务
        (*(task.function))(task.argument);
        // 执行完任务进入下一个循环
    }

    pool->started -= 1;// 线程池正在关闭，启动的线程数减1

    pthread_exit(nullptr);// 线程返回
    //return nullptr;
}

threadpool_t* threadpool_create(int thread_count, int queue_size)
{
    threadpool_t* pool = nullptr;

    // 编程技巧，实现goto
    do
    {
        // 参数不合法，退出
        if(thread_count <= 0 || thread_count > MAX_THREAD_COUNT || queue_size <= 0 || queue_size > MAX_QUEUE)
            return nullptr;

        // 分配线程池内存,失败则退出
        if((pool = (threadpool_t *)malloc(sizeof(threadpool_t))) == nullptr)
            break;

        // 动态初始化互斥量、条件变量, 假设一定初始化成功
        pthread_mutex_init(&(pool->lock), nullptr);
        pthread_cond_init(&(pool->notify), nullptr);

        // 为线程id数组分配内存
        pool->threads = (pthread_t *)malloc(sizeof(pthread_t) * thread_count);
        if(pool->threads == nullptr)
            break;
        //为task_queue分配内存
        pool->task_queue = (threadpool_task_t *)malloc(sizeof(threadpool_task_t) * queue_size);
        if(pool->task_queue == nullptr)
            break;

        // threadpool_t结构体初始化
        pool->threadCount = 0;// 下面for循环会添加线程，threadCount才会++
        pool->queue_size = queue_size;
        pool->head = 0;
        pool->tail = 0;
        pool->count = 0;
        pool->shutdown = 0;
        pool->started = 0;

        // 开启工作线程
        for (int i = 0; i < thread_count; ++i)
        {
            // 系统调用 ptherad_create() 创建线程，新线程通过调用带有参数的threadpool_thread函数开始运行
            if(pthread_create(&(pool->threads[i]), nullptr, threadpool_thread, (void*)pool) != 0)
            {// 销毁线程池
                threadpool_destory(pool,0);
                break;
            }
            pool->threadCount++;
            pool->started++;
        }
        // 全部成功，返回线程池pool
        return pool;

    }while (false);

    // 出现错误，释放已经申请的内存
    while(threadpool_free(pool));

    return nullptr;
}

int threadpool_add(threadpool_t *pool, void (*function)(void*), void* argument)
{
    int err = 0;

    if(pool == nullptr || function == nullptr)
    {
        return THREADPOOL_INVALID;
    }
    // 加锁
    if(pthread_mutex_lock(&pool->lock) != 0)
    {
        return THREADPOOL_LOCK_FAILURE;
    }

    int next = (pool->tail + 1) % pool->queue_size;// 避免数组越界，循环使用任务数组空间。
    do
    {
        // 挂起任务数量超过任务队列限制，无法再添加任务
        if(pool->count == pool->queue_size)
        {
            err = THREADPOOL_QUEUE_FULL;
            break;
        }
        // Are we shutting down?
        if(pool->shutdown)
        {
            err = THREADPOOL_SHUTDOWN;
            break;
        }
        // 将任务添加到任务队列
        pool->task_queue[pool->tail].function = function;
        pool->task_queue[pool->tail].argument = argument;
        pool->tail = next;
        pool->count += 1;
        // 通知消费者有任务了
        if(pthread_cond_signal(&(pool->notify)) != 0)
        {
            err = THREADPOOL_LOCK_FAILURE;
            break;
        }
    }while (false);

    // 解锁
    if(pthread_mutex_unlock(&pool->lock) != 0)
    {
        err = THREADPOOL_LOCK_FAILURE;
    }
    return err;
}

int threadpool_destory(threadpool_t *pool, int flags)
{
    printf("Thread pool destory");
    int err = 0;

    if(pool == nullptr)
        return THREADPOOL_INVALID;

    if(pthread_mutex_lock(&(pool->lock)) != 0)
    {
        return THREADPOOL_LOCK_FAILURE;
    }

    do
    {
        // already shutting down
        if(pool->shutdown)
        {
            err = THREADPOOL_SHUTDOWN;
            break;
        }

        pool->shutdown = (flags & THREADPOOL_GRACEFUL) ? graceful_shutdown : immediate_shutdown;

        // Wake up all worker threads
        if(pthread_cond_broadcast(&(pool->notify)) != 0 || pthread_mutex_unlock(&(pool->lock)) != 0)
        {// 若是broadcast失败，就会执行unlock
            err = THREADPOOL_LOCK_FAILURE;
            break;
        }
        // join all worker threads
        for(int i = 0; i < pool->threadCount; i++)
        {
            if(pthread_join(pool->threads[i], nullptr) != 0)
            {
                err = THREADPOOL_THREAD_FAILURE;
            }
        }
    }while(false);

    // Only if everything went well do we deallocate the pool
    if(!err)
    {
        threadpool_free(pool);
    }
    return err;
}


int threadpool_free(threadpool_t *pool)
{
    // 不需要释放
    if(pool == nullptr)
        return 0;

    if(pool->started > 0)
    {// 若是还有正在运行的线程，也不会释放内存
        return -1;
    }

    // threadpool_create申请内存时可能有部分是成功的，所以free前要判断一下。
    if(pool->threads)
        free(pool->threads);
    if(pool->task_queue)
        free(pool->task_queue);

    // 有点危险，无法确定mutex和cond是否已经初始化，就尝试释放
    pthread_mutex_destroy(&(pool->lock));   // 释放互斥量
    pthread_cond_destroy(&(pool->notify));  // 释放条件变量

    free(pool);
    return 0;
}

