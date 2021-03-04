/***********************************************************
 *@author RedDragon
 *@date 2021/3/2
 *@brief 这个锁专门针对requestData队列
 * 保证req_queue不会同时被两个工作线程修改
***********************************************************/

#ifndef WEBSERVER_LOCK_H
#define WEBSERVER_LOCK_H

#include <pthread.h>

// RAII
class Lock
{
private:
    static pthread_mutex_t mtx;
public:
    explicit Lock();
    ~Lock();
    // 删除
    Lock(const Lock&) = delete;
    Lock(Lock&&) = delete;
    Lock& operator=(const Lock&) = delete;
};


#endif //WEBSERVER_LOCK_H
