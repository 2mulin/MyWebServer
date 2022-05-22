#ifndef WEBSERVER_SINGLETON_H
#define WEBSERVER_SINGLETON_H

#include <boost/noncopyable.hpp>
#include <pthread.h>

template<class T>
class Singleton : boost::noncopyable
{
public:
    /**
     * @brief 返回唯一单例的引用，为什么不是指针？防止憨憨用户对实例指针delete。
     */
    static T& getInstance()
    {
        pthread_once(&m_once, init);
        return *m_instance;
    }

private:
    Singleton() = default;
    /// 私有化析构函数，这样就只能在heap上创建实例
    ~Singleton() = default;

    static void init()
    {
        m_instance = new T();
    }

private:
    static T* m_instance;
    static pthread_once_t m_once;
};

// 类中静态变量类中申明，类外初始化

template<typename T>
T* Singleton<T>::m_instance = nullptr;

template<typename T>
pthread_once_t Singleton<T>::m_once = PTHREAD_ONCE_INIT;

#endif //WEBSERVER_SINGLETON_H
