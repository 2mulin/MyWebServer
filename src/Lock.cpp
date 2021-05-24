/**
 *@author RedDragon
 *@date 2021/5/23
 *@brief RAII锁的实现
 */

#include "Lock.h"

Mutex::Mutex()
{
    pthread_mutex_init(&m_mutex, nullptr);
}
Mutex::~Mutex()
{
    pthread_mutex_destroy(&m_mutex);
}

void Mutex::lock()
{
    pthread_mutex_lock(&m_mutex);
}

void Mutex::unlock()
{
    pthread_mutex_unlock(&m_mutex);
}

/// 局部互斥量

ScopedLock::ScopedLock(Mutex& mtx)
    :m_mutex(mtx)
{
    m_mutex.lock();
    isLock = true;
}

void ScopedLock::lock()
{
    if(!isLock)
    {
        m_mutex.lock();
        isLock = true;
    }
}

void ScopedLock::unlock()
{
    if(isLock)
    {
        m_mutex.unlock();
        isLock = false;
    }
}

ScopedLock::~ScopedLock()
{
    unlock();
}

/// 读写互斥量

RWMutex::RWMutex()
{
    pthread_rwlock_init(&rwlock, nullptr);
}

RWMutex::~RWMutex()
{
    pthread_rwlock_destroy(&rwlock);
}

void RWMutex::readLock()
{
    pthread_rwlock_rdlock(&rwlock);
}
void RWMutex::writeLock()
{
    pthread_rwlock_wrlock(&rwlock);
}
void RWMutex::unlock()
{
    pthread_rwlock_unlock(&rwlock);
}

/// 局部读RAII

ReadScopedLock::ReadScopedLock(RWMutex& rwMutex)
    :mtx(rwMutex)
{
    mtx.readLock();
    isLock = true;
}

ReadScopedLock::~ReadScopedLock()
{
    unlock();
}

void ReadScopedLock::unlock()
{
    if(isLock)
    {
        mtx.unlock();
        isLock = false;
    }
}

void ReadScopedLock::Lock()
{
    if(!isLock)
    {
        mtx.readLock();
        isLock = true;
    }
}

/// 局部写RAII

WriteScopedLock::WriteScopedLock(RWMutex &rwMutex)
    :mtx(rwMutex)
{
    mtx.writeLock();
    isLock = true;
}

WriteScopedLock::~WriteScopedLock()
{
    unlock();
}

void WriteScopedLock::Lock()
{
    if(!isLock)
    {
        mtx.writeLock();
        isLock = true;
    }
}

void WriteScopedLock::unlock()
{
    if(isLock)
    {
        mtx.unlock();
        isLock = false;
    }
}