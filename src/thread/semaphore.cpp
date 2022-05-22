#include "thread/semaphore.h"

#include <stdexcept>

namespace WebServer
{
    Semaphore::Semaphore(uint32_t count)
    {
        if(sem_init(&m_semaphore, 0, count))
        {
            throw std::logic_error("sem_init failed!");
        }
    }

    Semaphore::~Semaphore()
    {
        sem_destroy(&m_semaphore);
    }

    void Semaphore::wait() {
        if(sem_wait(&m_semaphore)) {
            throw std::logic_error("sem_wait failed!");
        }
    }

    void Semaphore::notify() {
        if(sem_post(&m_semaphore)) {
            throw std::logic_error("sem_post failed!");
        }
    }
}