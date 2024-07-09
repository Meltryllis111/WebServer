#include "include/http_conn.h"

enum EpollMode
{
    LT,
    ET
};

// 添加文件描述符到epoll
void addfd(int epollfd, int fd, bool one_shot, EpollMode mode)
{

    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLRDHUP;
    if (mode == ET)
    {
        event.events |= EPOLLET;
    }
    if (one_shot)
    {
        event.events |= EPOLLONESHOT;
    }
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
}

// 从epoll删除文件描述符
void removefd(int epollfd, int fd)
{
    epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, 0);
}

// 修改文件描述符在epoll中的事件,重置EPOLLONESHOT事件,保证下一次可读时,EPOLLIN事件能够触发
void modfd(int epollfd, int fd, int ev)
{
    epoll_event event;
    event.data.fd = fd;
    event.events = ev | EPOLLRDHUP | EPOLLONESHOT;
    epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &event);
}