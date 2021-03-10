/***********************************************************
 *@author RedDragon
 *@date 2021/3/2
 *@brief RAII锁
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
    // 不可拷贝
    Lock(const Lock&) = delete;
    Lock(Lock&&) = delete;
    Lock& operator=(const Lock&) = delete;
};


#endif //WEBSERVER_LOCK_H
