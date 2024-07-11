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
#include <memory>
#include "threadpool.h"
// #include "locker.h"
#include "http_conn.h"
#include "inih/INIReader.h"
#include <spdlog/spdlog.h>

constexpr int MAX_CLIENT_NUM = 10000;   // 最大的客户端个数
constexpr int MAX_EVENT_NUMBER = 10000; // 最大的事件数

// 添加信号捕捉
void addSig(int sig, void (*handler)(int))
{
    struct sigaction sa;
    memset(&sa, '\0', sizeof(sa));
    sa.sa_handler = handler;
    sigfillset(&sa.sa_mask);
    sigaction(sig, &sa, nullptr);
}

// 读取配置文件，获取epoll模式
EpollMode getEpollMode(const std::string &filename)
{
    INIReader reader(filename);

    if (reader.ParseError() != 0)
    {
        throw std::runtime_error("无法读取配置文件");
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
        throw std::runtime_error("epoll模式配置错误,请检查配置文件");
    }
}

// 添加文件描述符到epoll
extern void addFd(int epollfd, int fd, bool one_shot, EpollMode mode);
// 从epoll删除文件描述符
extern void removeFd(int epollfd, int fd);
// // 修改文件描述符在epoll中的事件
// extern void modFd(int epollfd, int fd, int ev);

// 实现 make_unique 函数
template <typename T, typename... Args>
std::unique_ptr<T> make_unique(Args &&...args)
{
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

void setupLogger()
{
    spdlog::set_level(spdlog::level::debug);            // 设置全局日志等级
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S] [%l] %v"); // 设置日志格式
}

int main(int argc, char *argv[])
{
    setupLogger();

    // if (argc <= 1) {
    //     spdlog::error("参数错误,没有端口号");
    //     return -1;
    // }

    EpollMode mode = getEpollMode("../config.ini");

    // int port = std::stoi(argv[1]);
    int port = std::stoi("12345");
    spdlog::info("服务器启动，监听端口: {}", port);

    // 对SIGPIPE信号进行处理，防止因为客户端断开连接导致服务器崩溃
    addSig(SIGPIPE, SIG_IGN);

    // 创建线程池
    std::unique_ptr<ThreadPool<HttpConnection>> pool;
    try
    {
        pool = make_unique<ThreadPool<HttpConnection>>();
    }
    catch (const std::exception &e)
    {
        spdlog::error("创建线程池异常: {}", e.what());
        return -1;
    }

    // 创建一个数组保存客户端信息
    // std::unique_ptr<HttpConnection[]> users(new HttpConnection[MAX_CLIENT_NUM]);
    // std::vector<std::shared_ptr<HttpConnection>> users(MAX_CLIENT_NUM);
    HttpConnection *users = new HttpConnection[MAX_CLIENT_NUM];
    // 创建一个socket
    int listenfd = socket(PF_INET, SOCK_STREAM, 0);
    if (listenfd < 0)
    {
        spdlog::error("套接字错误");
        return -1;
    }

    // 设置端口复用
    int opt = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // 绑定端口
    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(listenfd, (sockaddr *)&address, sizeof(address)) < 0)
    {
        spdlog::error("绑定错误");
        return -1;
    }

    // 监听
    listen(listenfd, 5);
    spdlog::info("监听套接字: {}", listenfd);

    // 创建epoll对象，事件数组，添加事件。实现IO多路复用
    epoll_event events[MAX_EVENT_NUMBER];
    int epollfd = epoll_create(5);
    if (epollfd == -1)
    {
        spdlog::error("创建 epoll 实例失败: {}", strerror(errno));
        return -1;
    }
    spdlog::info("创建 epoll 实例成功, 文件描述符: {}", epollfd);
    // 将文件描述符添加到epoll内核事件表中
    addFd(epollfd, listenfd, false, mode);
    HttpConnection::epollFd = epollfd;

    HttpConnection *conn;
    while (true)
    {
        spdlog::info("等待事件发生");
        int num = epoll_wait(epollfd, events, MAX_EVENT_NUMBER, -1);
        if ((num < 0) && (errno != EINTR))
        {
            spdlog::error("epoll 错误");
            break;
        }

        for (int i = 0; i < num; i++)
        {
            int sockfd = events[i].data.fd;

            if (sockfd == listenfd)
            {
                sockaddr_in client_address;
                socklen_t client_addrlength = sizeof(client_address);
                int connfd = accept(listenfd, (sockaddr *)&client_address, &client_addrlength);
                if (connfd < 0)
                {
                    spdlog::warn("连接错误");
                    continue;
                }

                if (HttpConnection::userCount >= MAX_CLIENT_NUM)
                {
                    spdlog::warn("连接数已满，关闭新连接");
                    close(connfd);
                    continue;
                }
                spdlog::info("新连接: {}", connfd);
                users[connfd].init(connfd, client_address, mode);
            }
            else if (events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR))
            {
                spdlog::info("连接断开: {}", sockfd);
                users[sockfd].closeConnection();
            }
            else if (events[i].events & EPOLLIN)
            {
                spdlog::info("数据可读: {}", sockfd);
                if (users[sockfd].read())
                {
                    conn = &users[sockfd];
                    spdlog::info("加入任务队列: {}", sockfd);
                    pool->appendTask(users + sockfd);
                }
                else
                {
                    users[sockfd].closeConnection();
                }
            }
            else if (events[i].events & EPOLLOUT)
            {
                if (conn == &users[sockfd])
                {
                    spdlog::info("匹配成功");
                }
                else
                {
                    spdlog::info("匹配失败");
                }

                spdlog::info("开始写: {} {}", sockfd, users[sockfd].writeIndex);
                if (!users[sockfd].write())
                {
                    users[sockfd].closeConnection();
                }
            }
        }
    }

    close(epollfd);
    close(listenfd);
    return 0;
}
