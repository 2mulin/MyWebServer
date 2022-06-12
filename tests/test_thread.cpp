/**
 * @brief   测试thread的接口
 * 1. thread创建的线程是否正常
 * 2. 线程池接口是否正常,返回future是否可用.
 */

#include <iostream>
#include "thread/thread.h"
#include "thread/threadpool.h"
#include "thread/mutex.h"

using WebServer::Thread;

WebServer::Mutex mtx;

int print(int id)
{
    WebServer::ScopedLock<WebServer::Mutex> lk(mtx);
    std::cout << Thread::GetName() << " printf id: " << id  << ", thread id:" << Thread::GetID() << std::endl;
    lk.unlock();
    return id;
}

int main()
{
    ThreadPool *pool = new ThreadPool(4);

    std::vector<std::future<int>> result;

    for(int i = 1; i < 20; ++i){
        std::future<int> res = pool->addTask(print, i);
        result.push_back(std::move(res));
    }
    sleep(10);
    delete pool;

    WebServer::Thread t([](){print(1);}, "test_name");

    WebServer::ScopedLock<WebServer::Mutex> lk(mtx);
    std::cout << "name=" << t.getName() << ",ID=" << t.getID() << std::endl;
    lk.unlock();

    lk.lock();
    /// 这里的输出GetName输出的“main”，因为这是非WebServer::thread调用的。
    std::cout << "name=" << t.GetName() << ",ID=" << t.getID() << std::endl;
    lk.unlock();

    if(t.GetThis() == NULL)
        std::cout << "GetThis() success!" << std::endl;

    /// 等待所有任务完成，结果
    for(auto& fu : result)
    {
        if(fu.valid())
            std::cout << fu.get() << std::endl;
    }

    return 0;
}
