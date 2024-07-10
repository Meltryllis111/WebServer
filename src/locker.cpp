#include "locker.h"

// 互斥锁类实现
locker::locker() {
    if (pthread_mutex_init(&p_mutex, NULL) != 0) {
        throw std::runtime_error("互斥锁初始化失败");
    }
}

locker::~locker() {
    pthread_mutex_destroy(&p_mutex);
}

bool locker::lock() {
    return pthread_mutex_lock(&p_mutex) == 0;
}

bool locker::unlock() {
    return pthread_mutex_unlock(&p_mutex) == 0;
}

pthread_mutex_t* locker::get() {
    return &p_mutex;
}

// 条件变量类实现
cond::cond() {
    if (pthread_cond_init(&p_cond, NULL) != 0) {
        throw std::runtime_error("条件变量初始化失败");
    }
}

cond::~cond() {
    pthread_cond_destroy(&p_cond);
}

bool cond::wait(pthread_mutex_t* mutex) {
    return pthread_cond_wait(&p_cond, mutex) == 0;
}

bool cond::timedWait(pthread_mutex_t* mutex, const timespec* timeout) {
    return pthread_cond_timedwait(&p_cond, mutex, timeout) == 0;
}

bool cond::signal() {
    return pthread_cond_signal(&p_cond) == 0;
}

bool cond::broadcast() {
    return pthread_cond_broadcast(&p_cond) == 0;
}

// 信号量类实现
sem::sem() {
    if (sem_init(&p_sem, 0, 0) != 0) {
        throw std::runtime_error("信号量初始化失败");
    }
}

sem::sem(int num) {
    if (sem_init(&p_sem, 0, num) != 0) {
        throw std::runtime_error("信号量初始化失败");
    }
}

sem::~sem() {
    sem_destroy(&p_sem);
}

bool sem::p() {
    return sem_wait(&p_sem) == 0;
}

bool sem::v() {
    return sem_post(&p_sem) == 0;
}
