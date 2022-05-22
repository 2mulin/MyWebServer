/**
 * @date 2022/5/14
 * @brief 封装互斥量
 */
#ifndef WEBSERVER_MUTEX_H
#define WEBSERVER_MUTEX_H

#include <pthread.h>
#include <boost/noncopyable.hpp>

namespace WebServer{

    /**
     * @brief 简单封装pthread_mutex_t互斥量。
     */
    class Mutex : private boost::noncopyable
    {
    private:
        pthread_mutex_t m_mutex;

    public:
        explicit Mutex()
        {
            pthread_mutex_init(&m_mutex, NULL);
        }

        ~Mutex()
        {
            pthread_mutex_destroy(&m_mutex);
        }

        void lock()
        {
            pthread_mutex_lock(&m_mutex);
        }

        void unlock()
        {
            pthread_mutex_unlock(&m_mutex);
        }
    };

    /**
     * @brief 读写互斥量简单封装
     */
    class RWMutex: private boost::noncopyable
    {
    public:
        explicit RWMutex()
        {
            pthread_rwlock_init(&m_rwlock, NULL);
        }

        ~RWMutex()
        {
            pthread_rwlock_destroy(&m_rwlock);
        }

        void readLock()
        {
            pthread_rwlock_rdlock(&m_rwlock);
        }

        void writeLock()
        {
            pthread_rwlock_wrlock(&m_rwlock);
        }

        void unlock()
        {
            pthread_rwlock_unlock(&m_rwlock);
        }

    private:
        pthread_rwlock_t m_rwlock;
    };

    /**
     * @brief 局部互斥量，RAII锁的实现。
     * 构造的时候加锁，析构的时候解锁。还额外提供lock和unlock函数提供灵活性，还能尽量避免重复加锁，解锁。
     */
    template<typename T>
    class ScopedLock : private boost::noncopyable
    {
    private:
        T&      m_mutex;
        bool    m_isLock;

    public:
        explicit ScopedLock(T& mtx)
            :m_mutex(mtx)
        {
            m_mutex.lock();
            m_isLock = true;
        };
        ~ScopedLock()
        {
            this->unlock();
        }

        /**
         * @brief 加锁(若是已经加锁，直接返回)
         */
        void lock()
        {
            if(!m_isLock)
            {
                m_mutex.lock();
                m_isLock = true;
            }
        }
        /**
         * @brief 解锁(若是已经解锁，直接返回)
         */
        void unlock()
        {
            if(m_isLock)
            {
                m_mutex.unlock();
                m_isLock = false;
            }
        }
    };

    /**
     * @brief 局部读RAII锁实现
     */
    template<typename T>
    class ReadScopedLock : private boost::noncopyable
    {
    private:
        T&      m_lk;
        bool    m_isLock;

    public:
        explicit ReadScopedLock(T& rwMutex)
            :m_lk(rwMutex)
        {
            m_lk.readLock();
            m_isLock = true;
        }

        ~ReadScopedLock()
        {
            m_lk.unlock();
        }

        void Lock()
        {
            if(!m_isLock)
            {
                m_lk.readLock();
                m_isLock = true;
            }
        }

        void unlock()
        {
            if(m_isLock)
            {
                m_lk.unlock();
                m_isLock = false;
            }
        }
    };

    /**
     * @brief 局部写锁RAII实现
     */
    template<typename T>
    class WriteScopedLock : private boost::noncopyable
    {
    private:
        T&      m_mtx;
        bool    m_isLock;

    public:
        explicit WriteScopedLock(T& rwMutex)
            :m_mtx(rwMutex)
        {
            m_mtx.writeLock();
            m_isLock = true;
        }

        ~WriteScopedLock()
        {
            unlock();
        }

        void Lock()
        {
            if(!m_isLock)
            {
                m_mtx.writeLock();
                m_isLock = true;
            }
        }

        void unlock()
        {
            if(m_isLock)
            {
                m_mtx.unLock();
                m_isLock = false;
            }
        }
    };
}

#endif //WEBSERVER_MUTEX_H
