#ifndef HTTPCONNECTION_H
#define HTTPCONNECTION_H

#include <sys/epoll.h>
#include "inih/INIReader.h"
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
enum EpollMode
{
    LT,
    ET
};

class http_conn
{
private:
    int p_sockfd;          // 该http连接的socket
    sockaddr_in p_address; // 该http连接的socket地址
public:
    static int p_epollfd;    // epoll文件描述符,所有socket上的事件都注册到同一个epoll内核事件表中
    static int p_user_count; // 统计用户数量
    http_conn(/* args */);
    ~http_conn();
    void process();                                                 // 处理客户请求
    void init(int sockfd, const sockaddr_in &addr, EpollMode mode); // 初始化新接受的连接
    void close_conn();                                              // 关闭连接
    bool read();                                                    // 非阻塞读
    bool write();                                                   // 非阻塞写
};

#endif