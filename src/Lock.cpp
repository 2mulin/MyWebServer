/***********************************************************
 *@author RedDragon
 *@date 2021/3/2
 *@brief 
***********************************************************/

#include "Lock.h"

Mutex::Mutex()
{
    pthread_mutex_init(&mtx, nullptr);
}
Mutex::~Mutex()
{
    pthread_mutex_destroy(&mtx);
}

void Mutex::lock()
{
    pthread_mutex_lock(&mtx);
}

void Mutex::unlock()
{
    pthread_mutex_unlock(&mtx);
}


rwLock::rwLock()
{
    pthread_rwlock_init(&lock, nullptr);
}
rwLock::~rwLock()
{
    pthread_rwlock_destroy(&lock);
}
void rwLock::readLock()
{
    pthread_rwlock_wrlock(&lock);
}
void rwLock::writeLock()
{
    pthread_rwlock_wrlock(&lock);
}
void rwLock::unlock()
{
    pthread_rwlock_unlock(&lock);
}