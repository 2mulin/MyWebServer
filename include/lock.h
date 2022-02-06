/**
 * @author ling
 * @date 2021/5/23
 * @brief 简单封装一下pthread_mutex_t，以及实现RAII锁
 */

#ifndef WEBSERVER_LOCK_H
#define WEBSERVER_LOCK_H

#include <pthread.h>
#include "util/util.h"

/**
 * @brief 简单封装一下pthread_mutex_t这个互斥量。
 */
class Mutex : private nonCopyAble
{
private:
    pthread_mutex_t m_mutex;

public:
    explicit Mutex();
    void lock();
    void unlock();
    ~Mutex();
};

/**
 * @brief 局部互斥量，RAII锁的实现。
 * 构造的时候加锁，析构的时候解锁。还额外提供lock和unlock函数提供灵活性，还能尽量避免重复加锁，解锁。
 */
class ScopedLock : private nonCopyAble
{
private:
    Mutex& m_mutex;
    bool isLock;

public:
    explicit ScopedLock(Mutex& mtx);
    /**
     * @brief 加锁(若是已经加锁，直接返回)
     */
    void lock();
    /**
     * @brief 解锁(若是已经解锁，直接返回)
     */
    void unlock();
    ~ScopedLock();
};

/**
 * @brief 读写互斥量简单封装
 */
class RWMutex: private nonCopyAble
{
private:
    pthread_rwlock_t rwlock;

public:
    explicit RWMutex();
    ~RWMutex();
    void readLock();
    void writeLock();
    void unlock();
};

/**
 * @brief 局部读RAII锁实现
 */
class ReadScopedLock : private nonCopyAble
{
private:
    RWMutex& mtx;
    bool isLock;

public:
    explicit ReadScopedLock(RWMutex& rwMutex);
    ~ReadScopedLock();
    /**
     * @brief 加读锁
     */
    void Lock();
    void unlock();
};

/**
 * @brief 局部写锁RAII实现
 */
class WriteScopedLock : private nonCopyAble
{
private:
    RWMutex& mtx;
    bool isLock;

public:
    explicit WriteScopedLock(RWMutex& rwMutex);
    ~WriteScopedLock();
    /**
     * @brief 加写锁
     */
     void Lock();
     void unlock();
};


#endif //WEBSERVER_LOCK_H
