/***********************************************************
 *@author RedDragon
 *@date 2021/2/28
 *@brief 计时器
***********************************************************/
#ifndef WEBSERVER_TIMER_H
#define WEBSERVER_TIMER_H
#include "httpData.h"

// 计时器用于计时, 处理非活跃连接(断开他)
class Timer
{
public:
    long long expiredTime;
    httpData* data;
    Timer* next;
    Timer* prev;

    Timer();
    Timer(long long timeout);
    ~Timer();
    // 更新（加长）expired_time
    void update(long long timeout);
};

#endif //WEBSERVER_TIMER_H
