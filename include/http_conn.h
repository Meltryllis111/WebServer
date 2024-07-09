#ifndef HTTPCONNECTION_H
#define HTTPCONNECTION_H

#include <sys/epoll.h>

class http_conn
{
private:
    /* data */
public:
    http_conn(/* args */);
    ~http_conn();
    void process(); // 处理客户请求
};

http_conn::http_conn(/* args */)
{
}

http_conn::~http_conn()
{
}

#endif