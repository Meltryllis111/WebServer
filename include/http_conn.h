#ifndef HTTPCONNECTION_H
#define HTTPCONNECTION_H

#include <sys/epoll.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <cstring>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdarg.h>
#include <sys/uio.h>

#define READ_BUFFER_SIZE 4096
#define WRITE_BUFFER_SIZE 4096
#define FILENAME_LEN 200

enum EpollMode
{
    LT,
    ET
};

class http_conn
{
public:
    // 有限状态机的状态
    // HTTP请求方法，这里只支持GET
    enum METHOD
    {
        GET = 0,
        POST,
        HEAD,
        PUT,
        DELETE,
        TRACE,
        OPTIONS,
        CONNECT
    };
    // 解析客户端请求时，主状态机的状态
    enum CHECK_STATE
    {
        CHECK_STATE_REQUESTLINE = 0, // 请求行
        CHECK_STATE_HEADER,          // 头部字段
        CHECK_STATE_CONTENT          // 请求体
    };
    enum HTTP_CODE
    {
        NO_REQUEST,        // 请求不完整，需要继续读取请求数据
        GET_REQUEST,       // 获得了一个完整的客户请求
        BAD_REQUEST,       // 客户请求有语法错误
        NO_RESOURCE,       // 没有资源
        FORBIDDEN_REQUEST, // 客户对资源没有足够的访问权限
        FILE_REQUEST,      // 文件请求
        INTERNAL_ERROR,    // 服务器内部错误
        CLOSED_CONNECTION  // 客户端已经关闭连接
    };
    enum LINE_STATUS
    {
        LINE_OK = 0, // 读取到一个完整的行
        LINE_BAD,    // 行出错
        LINE_OPEN    // 行数据尚且不完整
    };

    // 定义HTTP响应的一些状态信息
    const char *ok_200_title = "OK";
    const char *error_400_title = "Bad Request";
    const char *error_400_form = "您的请求语法错误或本质上无法满足。\n";
    const char *error_403_title = "Forbidden";
    const char *error_403_form = "您没有权限从该服务器获取文件。\n";
    const char *error_404_title = "Not Found";
    const char *error_404_form = "该服务器上未找到所请求的文件。\n";
    const char *error_500_title = "Internal Error";
    const char *error_500_form = "提供所请求的文件时出现了异常问题。\n";

    // 网站的根目录
    const char *doc_root = "../resources";

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

    // 请求
    HTTP_CODE process_read();                      // 解析请求
    HTTP_CODE process_request_line(char *text);    // 解析请求行
    HTTP_CODE process_request_header(char *text);  // 解析请求头
    HTTP_CODE process_request_content(char *text); // 解析请求体
    LINE_STATUS parse_line();                      // 解析一行
    char *get_line();                              // 获取一行
    HTTP_CODE do_request();                        // 处理请求

    // 响应
    bool process_write(HTTP_CODE ret); // 生产响应内容
    void unmap();
    bool add_response(const char *format, ...);
    bool add_content(const char *content);
    bool add_content_type();
    bool add_status_line(int status, const char *title);
    bool add_headers(int content_length);
    bool add_content_length(int content_length);
    bool add_linger();
    bool add_blank_line();

private:
    int p_sockfd;          // 该http连接的socket
    sockaddr_in p_address; // 该http连接的socket地址

    char p_read_buf[READ_BUFFER_SIZE]; // 读缓冲区
    int p_read_idx;                    // 标识读缓冲区中已经读入的客户数据的最后一个字节的下一个位置
    int p_checked_idx;                 // 当前正在分析的字符在读缓冲区中的位置
    int p_start_line;                  // 当前正在解析的行的起始位置
    char *p_url;                       // 请求目标文件的文件名
    char *p_version;                   // 协议版本

    METHOD p_method;           // 请求方法
    CHECK_STATE p_check_state; // 主状态机当前所处的状态

    char *p_host;                   // 主机名
    bool p_linger;                  // 是否保持连接
    int p_content_length;           // 请求体长度
    char *p_content;                // 请求体内容
    const char *p_accept_encoding;  // Accept-Encoding字段
    char p_real_file[FILENAME_LEN]; // 客户请求的目标文件的完整路径，其内容等于doc_root + p_url
    void init();

    char p_write_buf[WRITE_BUFFER_SIZE]; // 写缓冲区
    int p_write_idx;                     // 写缓冲区中待发送的字节数
    char *p_file_address;                // 客户请求的目标文件被mmap到内存中的起始位置
    struct stat p_file_stat;             // 目标文件的信息
    struct iovec p_iv[2];                // 采用writev来执行写操作，定义两个iovec向量
    int p_iv_count;
};

#endif // HTTPCONNECTION_H
