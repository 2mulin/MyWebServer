/***********************************************************
 *@author RedDragon
 *@date 2021/2/28
 *@brief 计时器
***********************************************************/
#ifndef WEBSERVER_TIMER_H
#define WEBSERVER_TIMER_H
#include <cstdint>
#include <functional>
#include <queue>
#include <vector>
using std::priority_queue;
#include "Lock.h"

class TimerManager;
// 计时器
class Timer
{
    friend class TimerManager;
private:
    uint64_t timeout;                   // 超时时间
    std::function<void()> callBack;     // 回调函数

    // ms 秒后到期
    explicit Timer(uint64_t ms);
    Timer(uint64_t ms, std::function<void()> cb);
    ~Timer();

public:
    void cancel();      // 取消定时器
    // 还剩多久到期
    uint64_t getTimeOut()const
    {
        return timeout;
    }
};

struct Comparator{
    bool operator()(const Timer* t1, const Timer* t2)const
    {
        return t1->getTimeOut() > t2->getTimeOut();
    }
};

class TimerManager
{
private:
    RWLock lock;
    priority_queue<Timer*, std::vector<Timer*>, Comparator> Sequence;// 底层容器
    /* 底层容器保证按照超时时间升序排列
     * 选用堆 而不是 list的原因:
     * 首先时间复杂度: 堆               链表
     *  插入        O(logN)           O(N)
     *  删除   受限制(延迟删除O(n))     O(N)
     * 可以看出,大部分操作时间复杂度基本一致, 但是堆的插入是有优势
     */
public:
    TimerManager();
    ~TimerManager();
    // 添加定时器
    Timer* addTimer(uint64_t timeout, std::function<void()>&& cb);
    // 删除定时器(实际上没有删除,只是将cb置为null)
    void delTimer(Timer* ptr);
    // 执行所有到期事件
    void takeAllTimeout();
    // 得到最小的超时时间
    int64_t getMinTO();
};

#endif //WEBSERVER_TIMER_H
