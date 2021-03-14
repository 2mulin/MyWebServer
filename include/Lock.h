/***********************************************************
 *@author RedDragon
 *@date 2021/3/2
 *@brief RAII锁
***********************************************************/

#ifndef WEBSERVER_LOCK_H
#define WEBSERVER_LOCK_H

#include <pthread.h>

class nonCopyAble
{
public:
    nonCopyAble() = default;
    ~nonCopyAble() = default;
    // 禁用拷贝
    nonCopyAble(const nonCopyAble&) = delete;
    nonCopyAble& operator=(const nonCopyAble&) = delete;
};

// RAII
class Mutex : public nonCopyAble
{
private:
    pthread_mutex_t mtx;
public:
    explicit Mutex();
    void lock();
    void unlock();
    ~Mutex();
};


class RWLock: public nonCopyAble
{
private:
    pthread_rwlock_t lock;
public:
    explicit RWLock();
    ~RWLock();
    void readLock();
    void writeLock();
    void unlock();
};

template<class T>
struct ReadScopedLockImpl {
public:
    /**
     * @brief 构造函数
     * @param[in] mutex 读写锁
     */
    ReadScopedLockImpl(T& mutex)
            :m_mutex(mutex) {
        m_mutex.readLock();
        m_locked = true;
    }

    /**
     * @brief 析构函数,自动释放锁
     */
    ~ReadScopedLockImpl() {
        unlock();
    }

    /**
     * @brief 上读锁
     */
    void lock() {
        if(!m_locked) {
            m_mutex.readLock();
            m_locked = true;
        }
    }

    /**
     * @brief 释放锁
     */
    void unlock() {
        if(m_locked) {
            m_mutex.unlock();
            m_locked = false;
        }
    }
private:
    /// mutex
    T& m_mutex;
    /// 是否已上锁
    bool m_locked;
};

/**
 * @brief 局部写锁模板实现
 */
template<class T>
struct WriteScopedLockImpl {
public:
    /**
     * @brief 构造函数
     * @param[in] mutex 读写锁
     */
    WriteScopedLockImpl(T& mutex)
            :m_mutex(mutex) {
        m_mutex.writeLock();
        m_locked = true;
    }

    /**
     * @brief 析构函数
     */
    ~WriteScopedLockImpl() {
        unlock();
    }

    /**
     * @brief 上写锁
     */
    void lock() {
        if(!m_locked) {
            m_mutex.writeLock();
            m_locked = true;
        }
    }

    /**
     * @brief 解锁
     */
    void unlock() {
        if(m_locked) {
            m_mutex.unlock();
            m_locked = false;
        }
    }
private:
    /// Mutex
    T& m_mutex;
    /// 是否已上锁
    bool m_locked;
};

#endif //WEBSERVER_LOCK_H
