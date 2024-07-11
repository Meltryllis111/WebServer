#include "locker.h"

// 互斥锁类实现
Locker::Locker() 
    : mutex(new pthread_mutex_t, [](pthread_mutex_t* p) { pthread_mutex_destroy(p); delete p; }) {
    if (pthread_mutex_init(mutex.get(), nullptr) != 0) {
        throw std::runtime_error("Mutex initialization failed");
    }
}

bool Locker::lock() {
    return pthread_mutex_lock(mutex.get()) == 0;
}

bool Locker::unlock() {
    return pthread_mutex_unlock(mutex.get()) == 0;
}

pthread_mutex_t* Locker::get() {
    return mutex.get();
}

// 条件变量类实现
Condition::Condition()
    : cond(new pthread_cond_t, [](pthread_cond_t* p) { pthread_cond_destroy(p); delete p; }) {
    if (pthread_cond_init(cond.get(), nullptr) != 0) {
        throw std::runtime_error("Condition variable initialization failed");
    }
}

bool Condition::wait(pthread_mutex_t* mutex) {
    return pthread_cond_wait(cond.get(), mutex) == 0;
}

bool Condition::timedWait(pthread_mutex_t* mutex, const timespec* timeout) {
    return pthread_cond_timedwait(cond.get(), mutex, timeout) == 0;
}

bool Condition::signal() {
    return pthread_cond_signal(cond.get()) == 0;
}

bool Condition::broadcast() {
    return pthread_cond_broadcast(cond.get()) == 0;
}

// 信号量类实现
Semaphore::Semaphore() 
    : sem(new sem_t, [](sem_t* p) { sem_destroy(p); delete p; }) {
    if (sem_init(sem.get(), 0, 0) != 0) {
        throw std::runtime_error("Semaphore initialization failed");
    }
}

Semaphore::Semaphore(int num) 
    : sem(new sem_t, [](sem_t* p) { sem_destroy(p); delete p; }) {
    if (sem_init(sem.get(), 0, num) != 0) {
        throw std::runtime_error("Semaphore initialization failed");
    }
}

bool Semaphore::wait() {
    return sem_wait(sem.get()) == 0;
}

bool Semaphore::post() {
    return sem_post(sem.get()) == 0;
}
