/***********************************************************
 *@author RedDragon
 *@date 2020/9/5
 *@brief 
***********************************************************/

#ifndef WEBSERVER_THREADPOOL_H
#define WEBSERVER_THREADPOOL_H

#include <pthread.h>
#include "requestData.h"

const int THREADPOOL_INVALID = -1;// 线程不合法
const int THREADPOOL_LOCK_FAILURE = -2;// pthread_mutex_lock或unlock调用失败
const int THREADPOOL_QUEUE_FULL = -3;// 队列满
const int THREADPOOL_SHUTDOWN = -4;
const int THREADPOOL_THREAD_FAILURE = -5;
const int THREADPOOL_GRACEFUL = 1;

const int MAX_THREAD_COUNT = 1024;// 一个线程池的线程数最大限制为1024
const int MAX_QUEUE = 65535; // 一个任务队列最大任务数量限制


typedef enum
{
    immediate_shutdown = 1,// 立即
    graceful_shutdown  = 2// 优雅关闭
}threadpool_shutdown_t;


/**************************************
 * @struct threadpool_task
 * @brief 任务(函数指针和参数)
 *************************************/
struct threadpool_task_t
{
    void (*function)(void*);// 函数指针指向执行任务的函数
    void *argument;// 传递给函数的参数
};

/*****************************************
 * @struct threadpool_t
 * @brief 线程池结构体
 ****************************************/
struct threadpool_t
{
    pthread_mutex_t lock;// 互斥量
    pthread_cond_t notify;// 条件变量，用来通知worker thread
    pthread_t *threads;// 指向线程ID集合的指针

    int threadCount;// 线程数目

    threadpool_task_t *task_queue;// 任务队列，实际是一个数组
    int queue_size;// 任务队列大小
    // head和tail都是为了数组的index，head指向最前面未处理的任务，tail指向最后面的
    int head;
    int tail;

    int count;// 挂起任务数
    int shutdown;// 线程池是否正在关闭
    int started;// 已经启动的线程数
};

// 以指定固定线程数，任务队列大小创建一个线程池，启动所有线程，等待执行任务
threadpool_t* threadpool_create(int thread_count, int queue_size);
// 释放线程池的内存，若是线程池内还有线程在运行，则返回-1，成功释放返回0
int threadpool_free(threadpool_t *pool);

// 往线程池中增加一个任务（生产者）
int threadpool_add(threadpool_t *pool, void (*function)(void*), void* argument);
// join所有线程
int threadpool_destory(threadpool_t *pool, int flags);

#endif //WEBSERVER_THREADPOOL_H
