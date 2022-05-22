#include "thread/threadpool.h"

#include <cstring>

ThreadPool::ThreadPool(int thread_count)
    : m_isStop(false), m_threadCount(thread_count)
{
    // 每个线程执行的函数
    std::function<void()> func = [this](){
        while(true)
        {
            m_semaphore.wait();
            std::function<void()> task;
            {
                WebServer::ScopedLock<WebServer::Mutex> lk(m_mtx);
                if(m_taskQueue.empty())
                {
                    // 1. 任务全部执行完再退出, 将信号量清0
                    // 2. 如果线程池没有执行完任务,就析构了。 可能导致std::future不可用。 出现异常 std::future_error: Broken promise
                    if(m_isStop)
                        return ;
                    continue;
                }
                task = std::move(m_taskQueue.front());
                m_taskQueue.pop();
            }
            task();
        }

    };

    // 创建指定数目线程
    std::string name = "worker_";
    for(int i = 0; i < m_threadCount; ++i)
    {
        WebServer::Thread::ptr p = std::make_shared<WebServer::Thread>(func, name + std::to_string(i));
        m_vctThreads.push_back(p);
    }
}

ThreadPool::~ThreadPool()
{
    {
        WebServer::ScopedLock<WebServer::Mutex> lk(m_mtx);
        m_isStop = true;
    }
    for(int i = 0; i < m_threadCount; ++i)
        m_semaphore.notify();
    for(size_t i = 0; i < m_vctThreads.size(); ++i)
    {
        m_vctThreads[i]->join();
        --m_threadCount;
    }
    m_vctThreads.clear();
}