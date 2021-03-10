/***********************************************************
 *@author RedDragon
 *@date 2021/2/28
 *@brief 
***********************************************************/

#include "Timer.h"
#include <sys/time.h>
#include <cstdio>

Timer::Timer()
    : Timer(0)
{}

Timer::Timer(long long timeout)
    :data(nullptr),next(nullptr), prev(nullptr)
{
    struct timeval tv;
    int ret = gettimeofday(&tv, nullptr);
    if(ret == -1)
        perror("gettimeofday");
    expiredTime = timeout + (long long)tv.tv_sec * 1000 + (long long)tv.tv_usec / 1000;
}

Timer::~Timer()
{}

// 更新到期时间
void Timer::update(long long timeout)
{
    struct timeval tv;
    int ret = gettimeofday(&tv, nullptr);
    if(ret == -1)
        perror("gettimeofday");
    expiredTime = timeout + (long long)tv.tv_sec * 1000 + (long long)tv.tv_usec / 1000;
}

