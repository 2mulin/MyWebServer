/***********************************************************
 *@author RedDragon
 *@date 2021/2/28
 *@brief 
***********************************************************/
#ifndef WEBSERVER_TIMER_H
#define WEBSERVER_TIMER_H

#include <sys/time.h>
#include <cstdio>

// 计时器用于计时, 处理非活跃连接(断开他)
class timer
{
private:
    bool deleted;
    long long expiredTime;
    // requestData *request_data;
public:
    timer();
    timer(unsigned int timeout);
    ~timer();
    // 更新（加长）expired_time
    void update(unsigned int timeout);
    // 判断当前是否过期
    bool isExpired() const;
    // void clearReq();
    void setDeleted(){deleted = true;}
    // 包含的requestData对象是否被删除
    bool isDeleted() const{return deleted;}
    // 得到过期时间
    long long getExptime() const{return expiredTime;}
};

// 函数对象
struct timerCmp
{
    bool operator()(const timer *a, const timer *b) const
    {
        return a->getExptime() > b->getExptime();
    }
};

#endif //WEBSERVER_TIMER_H
