#include "http_conn.h"

int http_conn::p_epollfd = -1;
int http_conn::p_user_count = 0;

http_conn::http_conn(/* args */)
{
}
http_conn::~http_conn()
{
}
// 设置文件描述符为非阻塞
void setnonblocking(int fd)
{
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
}

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
    setnonblocking(fd);
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
// 初始化连接
void http_conn::init(int sockfd, const sockaddr_in &addr, EpollMode mode)
{
    p_sockfd = sockfd;
    p_address = addr;
    int opt = 1;
    setsockopt(p_sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    addfd(p_epollfd, p_sockfd, true, mode);
    p_user_count++; // 统计用户数量
}
// 关闭连接
void http_conn::close_conn()
{
    if (p_sockfd != -1)
    {
        removefd(p_epollfd, p_sockfd);
        close(p_sockfd);
        p_sockfd = -1;
        p_user_count--; // 统计用户数量
    }
}

// 读取客户端数据
bool http_conn::read()
{
    std::cout << "read" << std::endl;
    return true;
}

// 写数据
bool http_conn::write()
{
    std::cout << "write" << std::endl;
    return true;
}

// 处理客户请求的入口函数
void http_conn::process()
{
    //解析请求

    //生成响应
    std::cout << "process" << std::endl;
}