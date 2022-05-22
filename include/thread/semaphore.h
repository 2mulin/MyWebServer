/**
 * @brief 封装POSIX信号量
 */
#ifndef WEBSERVER_SEMAPHORE_H
#define WEBSERVER_SEMAPHORE_H

#include <cstdint>
#include <semaphore.h>

#include <boost/noncopyable.hpp>

namespace WebServer
{
    class Semaphore : boost::noncopyable
    {
    public:
        Semaphore(uint32_t count = 0);
        ~Semaphore();

        /**
         * @brief 等待信号量(-1)
         */
        void wait();

        /**
         * @brief 通知信号量(+1)
         */
        void notify();

    private:
        sem_t m_semaphore;
    };
}

#endif //WEBSERVER_SEMAPHORE_H
