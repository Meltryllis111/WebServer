#include <cstdio>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cerrno>
#include <exception>
#include <fcntl.h>
#include <sys/epoll.h>
#include <csignal>
#include "locker.h"
#include "threadpool.h"
#include "http_conn.h"
#include "inih/INIReader.h"

#define MAX_CLIENT_NUM 10000   // 最大的客户端个数
#define MAX_EVENT_NUMBER 10000 // 最大的事件数

enum EpollMode
{
    LT,
    ET
};

// 添加信号捕捉
void addSig(int sig, void (*hander)(int))
{
    struct sigaction sa;
    memset(&sa, '\0', sizeof(sa));
    sa.sa_handler = hander;
    sigfillset(&sa.sa_mask);
    sigaction(sig, &sa, NULL);
}
// 读取配置文件，获取epoll模式
EpollMode getEpollMode(const std::string &filename)
{
    INIReader reader(filename);

    if (reader.ParseError() != 0)
    {
        throw std::runtime_error("Unable to read configuration file.");
    }

    std::string mode = reader.Get("epoll", "mode", "LT");
    if (mode == "ET")
    {
        return EpollMode::ET;
    }
    else if (mode == "LT")
    {
        return EpollMode::LT;
    }
    else
    {
        throw std::runtime_error("Invalid epoll mode in configuration file.");
    }
}
// 添加文件描述符到epoll
extern void addfd(int epollfd, int fd, bool one_shot, EpollMode mode);
// 从epoll删除文件描述符
extern void removefd(int epollfd, int fd);
// 修改文件描述符在epoll中的事件
extern void modfd(int epollfd, int fd, int ev);

int main(int argc, char *argv[])
{
    EpollMode mode = getEpollMode("../config.ini");
    if (argc <= 1)
    {
        printf("usage: %s port_number\n", basename(argv[0]));
        exit(-1);
    }
    // 获取端口号
    int port = atoi(argv[1]);

    // 对SIGPIPE信号进行处理，防止因为客户端断开连接导致服务器崩溃
    addSig(SIGPIPE, SIG_IGN);

    // 创建线程池
    threadpool<http_conn> *pool = nullptr;
    try
    {
        pool = new threadpool<http_conn>;
    }
    catch (const std::exception &e) // 捕获标准异常
    {
        std::cerr << "Exception: " << e.what() << std::endl;
        return -1;
    }
    catch (...) // 其他异常
    {
        std::cerr << "Unknown exception occurred" << std::endl;
        exit(-1);
    }

    // 创建一个数组保存客户端信息
    http_conn *users = new http_conn[MAX_CLIENT_NUM];

    // 创建一个socket
    int listenfd = socket(PF_INET, SOCK_STREAM, 0);
    if (listenfd < 0)
    {
        std::cerr << "socket error" << std::endl;
        exit(-1);
    }

    // 设置端口复用
    int opt = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // 绑定端口
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    bind(listenfd, (struct sockaddr *)&address, sizeof(address));
    if (listenfd < 0)
    {
        std::cerr << "bind error" << std::endl;
        exit(-1);
    }

    // 监听
    listen(listenfd, 5); // backlog为5，定义了系统中挂起的连接队列的最大长度，半连接队列和全连接队列的总和

    // 创建epoll对象，事件数组，添加事件。实现IO多路复用
    epoll_event events[MAX_EVENT_NUMBER];
    int epollfd = epoll_create(5); // 传入的参数没有意义

    return 0;
}