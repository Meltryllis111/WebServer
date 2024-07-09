#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <list>
#include <cstdio>
#include "locker.h"
// 线程池类，通过模板类实现，模板参数T是任务类
template <typename T>
class threadpool
{
private:
    int p_thread_number;        // 线程池中的线程数
    pthread_t *p_threads;       // 线程池数组
    int p_max_requests;         // 请求队列中允许的最大请求数
    std::list<T *> p_workqueue; // 请求队列
    locker p_queuelocker;       // 保护请求队列的互斥锁
    sem p_queuestat;            // 信号量判断是否有任务需要处理
    bool p_stop;                // 是否结束线程
    static void *worker(void *arg);
    void run();

public:
    threadpool(int p_thread_number = 8, int p_max_requests = 10000);
    ~threadpool();
    bool appendTask(T *task); // 添加任务
};

template <typename T>
threadpool<T>::threadpool(int thread_number, int max_requests)
    : p_thread_number(thread_number),
      p_max_requests(max_requests),
      p_stop(false),
      p_threads(nullptr)
{
    if (thread_number <= 0 || p_max_requests <= 0)
    {
        throw std::runtime_error("Wrong thread pool parameters");
    }
    p_threads = new pthread_t[p_thread_number];
    if (!p_threads)
    {
        throw std::runtime_error("Failed to initialize thread pool");
    }

    // 创建指定数量的线程，并且设置为线程脱离
    for (int i = 0; i < thread_number; i++)
    {
        std::cout << "create the " << i << "th thread" << std::endl;

        if (pthread_create(p_threads + i, nullptr, worker, this) != 0)
        {
            delete[] p_threads;
            throw std::runtime_error("Failed to create thread");
        }
        if (pthread_detach(p_threads[i]))
        {
            delete[] p_threads;
            throw std::runtime_error("Failed to detach thread");
        }
    }
}
template <typename T>
threadpool<T>::~threadpool()
{
    p_stop = true;
    delete[] p_threads;
}

template <typename T>
bool threadpool<T>::appendTask(T *task)
{
    p_queuelocker.lock();
    if (p_workqueue.size() > p_max_requests)
    {
        p_queuelocker.unlock();
        return false;
    }
    p_workqueue.push_back(task);
    p_queuelocker.unlock();
    p_queuestat.v();
    return true;
}

template <typename T>
void *threadpool<T>::worker(void *args)
{
    threadpool *pool = (threadpool *)args;
    pool->run();
    return pool;
}
template <typename T>
void threadpool<T>::run()
{
    while (!p_stop)
    {
        p_queuestat.p();
        p_queuelocker.lock();
        if (p_workqueue.empty())
        {
            p_queuelocker.unlock();
            continue;
        }
        T *task = p_workqueue.front();
        p_workqueue.pop_front();
        p_queuelocker.unlock();
        if (!task)
        {
            continue;
        }
        task->process();
    }
}
#endif