/***********************************************************
 *@author RedDragon
 *@date 2021/3/2
 *@brief 
***********************************************************/

#include "Lock.h"

pthread_mutex_t Lock::mtx = PTHREAD_MUTEX_INITIALIZER;

Lock::Lock()
{
    pthread_mutex_lock(&mtx);
}

Lock::~Lock()
{
    pthread_mutex_unlock(&mtx);
}
