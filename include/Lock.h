/***********************************************************
 *@author RedDragon
 *@date 2021/3/2
 *@brief RAII锁
***********************************************************/

#ifndef WEBSERVER_LOCK_H
#define WEBSERVER_LOCK_H

#include <pthread.h>

class nonCopyAble
{
public:
    nonCopyAble() = default;
    ~nonCopyAble() = default;
    // 禁用拷贝
    nonCopyAble(const nonCopyAble&) = delete;
    nonCopyAble& operator=(const nonCopyAble&) = delete;
};

// RAII
class Mutex : public nonCopyAble
{
private:
    pthread_mutex_t mtx;
public:
    explicit Mutex();
    void lock();
    void unlock();
    ~Mutex();
};


class rwLock: public nonCopyAble
{
private:
    pthread_rwlock_t lock;
public:
    explicit rwLock();
    ~rwLock();
    void readLock();
    void writeLock();
    void unlock();
};

#endif //WEBSERVER_LOCK_H
