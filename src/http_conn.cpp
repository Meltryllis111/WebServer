#include "include/http_conn.h"


void addfd(int epollfd, int fd, bool one_shot)
{
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
}

void removefd(int epollfd, int fd)
{
}