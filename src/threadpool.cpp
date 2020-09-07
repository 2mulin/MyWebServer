/***********************************************************
 *@author RedDragon
 *@date 2020/9/5
 *@brief
***********************************************************/

#include "threadpool.h"

threadpool_t* threadpool_create(int thread_count, int queue_size, int flags)
{
    threadpool_t* pool;
    int i;
    do
    {
        if(thread_count <= 0 || thread_count > MAX_THREADS || queue_size <= 0 || queue_size > MAX_QUEUE)
            return nullptr;

        if((pool = (threadpool_t *)malloc(sizeof(threadpool_t))) == nullptr)
            break;

        // 初始化
        pool->thread_count = 0;
        pool->queue_size = queue_size;
        pool->head = 0;
        pool->tail = 0;
        pool->count = 0;

        // 为线程和queue分配空间
        pool->threads = (pthread_t *)malloc(sizeof(pthread_t) * thread_count);
        pool->queue = (threadpool_task_t *)malloc(sizeof(threadpool_task_t) * queue_size);

        // 若初始化互斥量和条件变量失败 或者 分配空间失败
        if(pthread_mutex_init(&(pool->lock), nullptr) != 0) ||
            (pthread_cond_init(&(pool->notify), nullptr) != 0) ||
            (pool->threads == nullptr) || (pool->queue == nullptr)
        {
            break;
        }

        // 开启工作线程
        for (int i = 0; i < thread_count; ++i)
        {
            if(pthread_create(&(pool->threads[i]), nullptr, threadpool_thread, (void*)pool) != 0)
            {
                
            }
        }
    }while (false);
}

int threadpool_add(threadpool_t *pool, void (*function)(void*))
{

}

int threadpool_destory(threadpool_t *pool, int flags)
{

}

int threadpool_free(threadpool_t *pool)
{

}

static void *threadpool_thread(void *threadpool)
{

}
