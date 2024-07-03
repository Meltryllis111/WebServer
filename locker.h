#ifndef LOCKER_H
#define LOCKER_H

#include <pthread.h>
#include <stdexcept>
#include <semaphore.h>
// 线程同步机制封装类

// 互斥锁类
class locker
{
private:
    pthread_mutex_t p_mutex;

public:
    locker();
    ~locker();
    bool lock();
    bool unlock();
    pthread_mutex_t *get();
};

locker::locker(/* args */)
{
    if (pthread_mutex_init(&p_mutex, NULL) != 0)
    {
        throw std::runtime_error("Failed to initialize mutex");
    }
}
locker::~locker()
{
    pthread_mutex_destroy(&p_mutex);
}
bool locker::lock()
{
    return pthread_mutex_lock(&p_mutex) == 0;
}
bool locker::unlock()
{
    return pthread_mutex_unlock(&p_mutex) == 0;
}
pthread_mutex_t *locker::get()
{
    return &p_mutex;
}

// 条件变量类
class cond
{
private:
    pthread_cond_t p_cond;

public:
    cond();
    ~cond();
    bool wait(pthread_mutex_t *mutex);
    bool timedWait(pthread_mutex_t *mutex, const timespec *timeout);
    bool signal();
    bool broadcast();
};

cond::cond()
{
    if (pthread_cond_init(&p_cond, NULL) != 0)
    {
        throw std::runtime_error("Failed to initialize condition variable");
    }
}
cond::~cond()
{
    pthread_cond_destroy(&p_cond);
}
bool cond::wait(pthread_mutex_t *mutex)
{
    return pthread_cond_wait(&p_cond, mutex) == 0;
}
bool cond::timedWait(pthread_mutex_t *mutex, const timespec *timeout)
{
    return pthread_cond_timedwait(&p_cond, mutex, timeout) == 0;
}

bool cond::signal()
{
    return pthread_cond_signal(&p_cond) == 0;
}

bool cond::broadcast()
{
    return pthread_cond_broadcast(&p_cond) == 0;
}

// 信号量类
class sem
{
private:
    sem_t p_sem;
public:
    sem();
    sem(int num);
    ~sem();
    bool p();
    bool v();

};

sem::sem()
{
    if (sem_init(&p_sem,0,0)!=0)
    {
        throw std::runtime_error("Failed to initialize semaphore");        
    }
}
sem::sem(int num)
{
    if (sem_init(&p_sem,0,num)!=0)
    {
        throw std::runtime_error("Failed to initialize semaphore");        
    }
}
sem::~sem()
{
    sem_destroy(&p_sem);
}
bool sem::p()
{
    return sem_wait(&p_sem)==0;
}
bool sem::v()
{
    return sem_post(&p_sem)==0;
}

#endif