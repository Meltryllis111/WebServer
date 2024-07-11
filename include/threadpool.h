#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <list>
#include <cstdio>
#include <vector>
#include <memory>
#include <iostream>
#include "locker.h"
#include <thread>
// 线程池类，通过模板类实现，模板参数T是任务类
template <typename T>
class ThreadPool
{
private:
    int thread_number;                // 线程池中的线程数
    int max_requests;                 // 请求队列中允许的最大请求数
    std::vector<std::thread> threads; // 线程池数组
    std::list<T *> workqueue;         // 请求队列
    Locker queue_locker;              // 保护请求队列的互斥锁
    Semaphore queue_stat;             // 信号量判断是否有任务需要处理
    bool stop;                        // 是否结束线程
    static void worker(ThreadPool *pool);
    void run();

public:
    ThreadPool(int thread_number = 8, int max_requests = 10000);
    ~ThreadPool();
    bool appendTask(T *task); // 添加任务
};

template <typename T>
ThreadPool<T>::ThreadPool(int thread_number, int max_requests)
    : thread_number(thread_number),
      max_requests(max_requests),
      stop(false)
{
    if (thread_number <= 0 || max_requests <= 0)
    {
        throw std::runtime_error("线程池初始化失败");
    }

    for (int i = 0; i < thread_number; i++)
    {
        std::cout << "创建第" << i << "个线程" << std::endl;
        threads.emplace_back(worker, this);
    }
}

template <typename T>
ThreadPool<T>::~ThreadPool()
{
    stop = true;
    for (std::thread &thread : threads)
    {
        if (thread.joinable())
        {
            thread.join();
        }
    }
}

template <typename T>
bool ThreadPool<T>::appendTask(T *task)
{
    queue_locker.lock();
    if (workqueue.size() > max_requests)
    {
        queue_locker.unlock();
        return false;
    }
    workqueue.push_back(task);
    queue_locker.unlock();
    queue_stat.post();
    return true;
}

template <typename T>
void ThreadPool<T>::worker(ThreadPool *pool)
{
    pool->run();
}

template <typename T>
void ThreadPool<T>::run()
{
    while (!stop)
    {
        queue_stat.wait();
        queue_locker.lock();
        if (workqueue.empty())
        {
            queue_locker.unlock();
            continue;
        }
        T *task = workqueue.front();
        workqueue.pop_front();
        queue_locker.unlock();
        if (!task)
        {
            continue;
        }
        task->process();
    }
}

#endif // THREADPOOL_H
