/***********************************************************
 *@author RedDragon
 *@date 2021/2/28
 *@brief
 * 使用gettimeofday()实现的毫秒级定时器.
***********************************************************/
#include "Timer.h"
#include "util.h"

// Timer的实现

Timer::Timer(uint64_t ms)
    : timeout(ms)
{
    timeout += getMilliSecond();
}

Timer::Timer(uint64_t ms, std::function<void()> cb)
    :timeout(ms), callBack(cb)
{
    timeout += getMilliSecond();
}

Timer::~Timer()
{}

void Timer::cancel()
{
    if(callBack)
        callBack = nullptr;
}

// TimerManager的实现

TimerManager::TimerManager()
{}
// 析构时删除所有timer
TimerManager::~TimerManager()
{
    WriteScopedLockImpl<RWLock> writeLock(lock);
    while(!Sequence.empty())
    {
        Timer* p = Sequence.top();
        delete p;
        Sequence.pop();
    }
}

// 复杂度O(lgN)
Timer* TimerManager::addTimer(uint64_t ms, std::function<void()>&& cb)
{
    Timer* p = new Timer(ms, cb);
    WriteScopedLockImpl<RWLock> wirteLock(lock);
    Sequence.push(p);
    return p;
}

// 由于堆只能删除栈顶元素, 所以这里使用延迟删除, 就是只有前面的
// 可以删除, 才会真正删除, 否则先取消计时器, 等待删除
void TimerManager::delTimer(Timer* p)
{
    WriteScopedLockImpl<RWLock> writeLock(lock);
    p->cancel();
}

// 执行所有超时事件
void TimerManager::takeAllTimeout()
{
    uint64_t now = getMilliSecond();
    WriteScopedLockImpl<RWLock> writeLock(lock);
    printf("执行清理, 还剩%lu个元素\n", Sequence.size());
    while(!Sequence.empty() && Sequence.top()->getTimeOut() < now)
    {
        Timer* p = Sequence.top();
        // 因为有些定时器是延迟删除, 所以会出现callback是null的定时器
        if(p->callBack != nullptr)
            p->callBack();// 执行定时任务
        Sequence.pop();
    }
}

// 最小到期时间还剩多久
int64_t TimerManager::getMinTO()
{
    ReadScopedLockImpl<RWLock> readLock(lock);
    if(Sequence.empty())
        return -1;
    return Sequence.top()->getTimeOut() - getMilliSecond();
}