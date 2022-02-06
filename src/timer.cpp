#include "timer.h"
#include "util/util.h"

Timer::Timer(uint64_t ms)
    : timeout(ms)
{
    timeout += current_time();
}

Timer::Timer(uint64_t ms, std::function<void()> cb)
    :timeout(ms), callBack(cb)
{
    timeout += current_time();
}

Timer::~Timer()
{}

void Timer::cancel()
{
    if(callBack)
        callBack = nullptr;
}

TimerManager::TimerManager()
{}

TimerManager::~TimerManager()
{
    WriteScopedLock writeLock(lock);
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
    WriteScopedLock wirteLock(lock);
    Sequence.push(p);
    return p;
}

// 由于堆只能删除栈顶元素, 所以这里使用延迟删除, 就是只有前面的
// 可以删除, 才会真正删除, 否则先取消计时器, 等待删除
void TimerManager::delTimer(Timer* p)
{
    WriteScopedLock writeLock(lock);
    p->cancel();
}

// 执行所有超时事件
void TimerManager::takeAllTimeout()
{
    WriteScopedLock writeLock(lock);
    uint64_t now = current_time();
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
    ReadScopedLock readLock(lock);
    if(Sequence.empty())
        return -1;
    return Sequence.top()->getTimeOut() - current_time();
}