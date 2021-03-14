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


RWLock::RWLock()
{
    pthread_rwlock_init(&lock, nullptr);
}
RWLock::~RWLock()
{
    pthread_rwlock_destroy(&lock);
}
void RWLock::readLock()
{
    pthread_rwlock_wrlock(&lock);
}
void RWLock::writeLock()
{
    pthread_rwlock_wrlock(&lock);
}
void RWLock::unlock()
{
    pthread_rwlock_unlock(&lock);
}