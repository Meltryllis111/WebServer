#ifndef LOCKER_H
#define LOCKER_H

#include <pthread.h>
#include <stdexcept>
#include <semaphore.h>

// 互斥锁类
class locker {
private:
    pthread_mutex_t p_mutex;

public:
    locker();
    ~locker();
    bool lock();
    bool unlock();
    pthread_mutex_t* get();
};

// 条件变量类
class cond {
private:
    pthread_cond_t p_cond;

public:
    cond();
    ~cond();
    bool wait(pthread_mutex_t* mutex);
    bool timedWait(pthread_mutex_t* mutex, const timespec* timeout);
    bool signal();
    bool broadcast();
};

// 信号量类
class sem {
private:
    sem_t p_sem;

public:
    sem();
    sem(int num);
    ~sem();
    bool p();
    bool v();
};

#endif // LOCKER_H
