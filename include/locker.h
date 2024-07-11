#ifndef LOCKER_H
#define LOCKER_H

#include <pthread.h>
#include <stdexcept>
#include <semaphore.h>
#include <memory>

// 互斥锁类
class Locker {
private:
    std::unique_ptr<pthread_mutex_t, void(*)(pthread_mutex_t*)> mutex;

public:
    Locker();
    ~Locker() = default;
    bool lock();
    bool unlock();
    pthread_mutex_t* get();
};

// 条件变量类
class Condition {
private:
    std::unique_ptr<pthread_cond_t, void(*)(pthread_cond_t*)> cond;

public:
    Condition();
    ~Condition() = default;
    bool wait(pthread_mutex_t* mutex);
    bool timedWait(pthread_mutex_t* mutex, const timespec* timeout);
    bool signal();
    bool broadcast();
};

// 信号量类
class Semaphore {
private:
    std::unique_ptr<sem_t, void(*)(sem_t*)> sem;

public:
    Semaphore();
    explicit Semaphore(int num);
    ~Semaphore() = default;
    bool wait();
    bool post();
};

#endif // LOCKER_H
