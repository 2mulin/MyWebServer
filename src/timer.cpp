/***********************************************************
 *@author RedDragon
 *@date 2021/2/28
 *@brief 
***********************************************************/

#include "timer.h"

// 默认500毫秒到期
timer::timer()
    : timer(500)
{}

timer::timer(unsigned int timeout)
{
    struct timeval tv;
    int ret = gettimeofday(&tv, nullptr);
    if(ret == -1)
        perror("gettimeofday");
    expiredTime = (long long)timeout + (long long)tv.tv_sec * 1000 + (long long)tv.tv_usec / 1000;
    printf("创建定时器!\n");
}

timer::~timer()
{
    printf("定时器销毁!\n");
}

// 更新到期时间
void timer::update(unsigned int timeout)
{
    struct timeval tv;
    int ret = gettimeofday(&tv, nullptr);
    if(ret == -1)
        perror("gettimeofday");
    expiredTime = (long long)timeout + (long long)tv.tv_sec * 1000 + (long long)tv.tv_usec / 1000;
}

// 判断是否到期
bool timer::isExpired()const
{
    struct timeval now;
    gettimeofday(&now, nullptr);
    long long nowTime = (long long)now.tv_sec * 1000 + (long long)now.tv_usec/1000;
    if(nowTime > expiredTime)
        return false;
    return true;
}