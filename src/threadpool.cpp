/***********************************************************
 *@author RedDragon
 *@date 2020/9/5
 *@brief
***********************************************************/

#include "threadpool.h"

threadpool_t* threadpool_create(int thread_count, int queue_size, int flags)
{
    threadpool_t* pool;
    do
    {
        if(thread_count <= 0 || thread_count > MAX_THREADS || queue_size <= 0 || queue_size > MAX_QUEUE)
            return nullptr;

        // 动态分配线程池内存
        if((pool = (threadpool_t *)malloc(sizeof(threadpool_t))) == nullptr)
            break;

        // 线程池初始化
        pool->thread_count = 0;
        pool->queue_size = queue_size;
        pool->head = 0;
        pool->tail = 0;
        pool->count = 0;
        pool->shutdown = pool->started = 0;

        // 为线程id和queue分配空间
        pool->threads = (pthread_t *)malloc(sizeof(pthread_t) * thread_count);
        pool->queue = (threadpool_task_t *)malloc(sizeof(threadpool_task_t) * queue_size);

        // 若初始化互斥量 或 初始化条件变量失败 或者 分配空间失败
        if(pthread_mutex_init(&(pool->lock), nullptr) != 0 ||
            (pthread_cond_init(&(pool->notify), nullptr) != 0) ||
            (pool->threads == nullptr) ||
            (pool->queue == nullptr))
        {
            break;
        }

        // 开启工作线程
        for (int i = 0; i < thread_count; ++i)
        {
            if(pthread_create(&(pool->threads[i]), nullptr, threadpool_thread, (void*)pool) != 0)
            {// 销毁线程池
                threadpool_destory(pool,0);
                break;
            }
            pool->thread_count++;
            pool->started++;
        }
        return pool;
    }while (false);

    if(pool != nullptr)
        threadpool_free(pool);

    return nullptr;
}

int threadpool_add(threadpool_t *pool, void (*function)(void*), void* argument, int flags)
{
    int err = 0;
    int next;

    if(pool == nullptr || function == nullptr)
    {
        return THREADPOOL_INVALID;
    }
    if(pthread_mutex_lock(&pool->lock) != 0)
    {
        return THREADPOOL_LOCK_FAILURE;
    }
    next = (pool->tail + 1) % pool->queue_size;

    // 利用循环机制，类似于goto
    do
    {
        // Are we full?
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
        // Add task to queue?
        pool->queue[pool->tail].function = function;
        pool->queue[pool->tail].argument = argument;
        pool->tail = next;
        pool->count += 1;
        // pthread_cond_boradcast()
        if(pthread_cond_signal(&(pool->notify)) != 0)
        {
            err = THREADPOOL_LOCK_FAILURE;
            break;
        }
    }while (false);

    if(pthread_mutex_unlock(&pool->lock) != 0)
    {
        err = THREADPOOL_LOCK_FAILURE;
    }
    return err;
}

int threadpool_destory(threadpool_t *pool, int flags)
{
    printf("Thread pool destory");
    int i, err = 0;

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

        pool->shutdown =
    }while(false);
}

int threadpool_free(threadpool_t *pool)
{

}

static void *threadpool_thread(void *threadpool)
{
    threadpool_t *pool = (threadpool_t *)threadpool;
    threadpool_task_t task;

    for(;;)
    {
        // 必须锁定才能执行 pthread_cond_wait()
        pthread_mutex_lock(&(pool->lock));

        // 等待条件变量，检查是否有虚假唤醒。当从pthread_cond_wait()返回时，我们拥有锁。
        while((pool->count == 0) && (!pool->shutdown))
        {// 若是pool->count > 0说明等到了
            pthread_cond_wait(&pool->notify, &pool->lock);
        }
        if((pool->shutdown == immediate_shutdown) ||
            ((pool->shutdown == graceful_shutdown) &&
            (pool->count == 0)))
        {
            break;
        }

        task.function = pool->queue[pool->head].function;
        task.argument = pool->queue[pool->head].argument;
        pool->head = (pool->head + 1) % pool->queue_size;
        pool->count -= 1;

        pthread_mutex_unlock(&(pool->lock));

        // 执行实际要做的任务
        ((*task.function))(task.argument);
    }

    // 这代码。。。。。
    --pool->started;// 启动的线程数减1

    // 这是因为上面的break退出时没unlock，可以放到break那里去
    pthread_mutex_unlock(&(pool->lock));
    pthread_exit(nullptr);

    return nullptr;
}
