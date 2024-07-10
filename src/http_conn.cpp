#include "http_conn.h"

int http_conn::p_epollfd = -1;
int http_conn::p_user_count = 0;

http_conn::http_conn(/* args */)
{
}
http_conn::~http_conn()
{
}

// 初始化连接的参数
void http_conn::init_args()
{
    p_check_state = CHECK_STATE_REQUESTLINE; // 初始状态为检查请求行
    p_linger = false;                        // 默认不保持链接  Connection : keep-alive保持连接

    p_method = GET; // 默认请求方式为GET
    p_url = 0;
    p_version = 0;
    p_content_length = 0;
    p_host = 0;
    p_start_line = 0;
    p_checked_idx = 0;
    p_read_idx = 0;
    p_write_idx = 0;
    bzero(p_read_buf, READ_BUFFER_SIZE);
    bzero(p_write_buf, READ_BUFFER_SIZE);
    bzero(p_real_file, FILENAME_LEN);
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
    p_file_address = nullptr;
    memset(p_write_buf, '\0', WRITE_BUFFER_SIZE); // 清空缓冲区
    init_args();
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

// 循环读取客户端数据
bool http_conn::read()
{
    if (p_read_idx >= READ_BUFFER_SIZE) // 缓冲区已满
    {
        std::cerr << "读缓冲区满了" << std::endl;
        return false;
    }
    int bytes_read = 0;
    while (true)
    {
        bytes_read = recv(p_sockfd, p_read_buf + p_read_idx, READ_BUFFER_SIZE - p_read_idx, 0);
        if (bytes_read == -1)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK) // 没有数据了
            {
                break;
            }
            return false;
        }
        else if (bytes_read == 0) // 对方关闭连接
        {
            perror("客户端关闭连接");
            return false;
        }
        p_read_idx += bytes_read;
        if (p_read_idx >= READ_BUFFER_SIZE)
        {
            // 防止缓冲区溢出
            std::cerr << "读缓冲区溢出" << std::endl;
            return false;
        }
    }
    return true;
}

// 写数据
bool http_conn::write()
{
    int temp = 0;
    int bytes_have_sent = 0;         // 已经发送的字节
    int bytes_to_send = p_write_idx; // 将要发送的字节数

    if (bytes_to_send == 0)
    {
        modfd(p_epollfd, p_sockfd, EPOLLIN);
        init_args(); // 初始化连接状态，清空缓冲区等
        return true;
    }

    while (1)
    {
        temp = writev(p_sockfd, p_iv, p_iv_count);
        if (temp <= -1)
        {
            if (errno == EAGAIN)
            {
                modfd(p_epollfd, p_sockfd, EPOLLOUT);
                return true;
            }
            unmap(); // 释放映射的内存区域
            return false;
        }
        bytes_to_send -= temp;
        bytes_have_sent += temp;

        if (bytes_to_send <= 0)
        {
            unmap(); // 释放映射的内存区域
            if (p_linger)
            {
                init_args(); // 初始化连接状态，清空缓冲区等
                modfd(p_epollfd, p_sockfd, EPOLLIN);
                return true;
            }
            else
            {
                modfd(p_epollfd, p_sockfd, EPOLLIN);
                return false;
            }
        }
    }
}

// 解析请求
http_conn::HTTP_CODE http_conn::process_read()
{
    LINE_STATUS line_status = LINE_OK;
    HTTP_CODE ret = NO_REQUEST;
    char *text = 0;

    while ((line_status = parse_line()) == LINE_OK)
    {
        text = get_line();
        p_start_line = p_checked_idx;
        switch (p_check_state)
        {
        case CHECK_STATE_REQUESTLINE:
        {
            ret = process_request_line(text);
            if (ret == BAD_REQUEST)
            {
                return BAD_REQUEST;
            }
            break;
        }
        case CHECK_STATE_HEADER:
        {
            ret = process_request_header(text);
            if (ret == BAD_REQUEST)
            {
                return BAD_REQUEST;
            }
            else if (ret == GET_REQUEST)
            {
                return do_request();
            }
            break;
        }
        case CHECK_STATE_CONTENT:
        {
            ret = process_request_content(text);
            if (ret == GET_REQUEST)
            {
                return do_request();
            }
            line_status = LINE_OPEN;
            break;
        }
        default:
        {
            return INTERNAL_ERROR;
        }
        }
    }

    return NO_REQUEST;
}

// 解析请求行
http_conn::HTTP_CODE http_conn::process_request_line(char *text)
{
    char *url = strpbrk(text, " \t"); // 查找空格或制表符，提取 URL。
    if (!url)
    {
        return BAD_REQUEST;
    }
    *url++ = '\0';

    char *method = text;
    if (strcasecmp(method, "GET") == 0)
    {
        p_method = GET;
    }
    else
    {
        return BAD_REQUEST;
    }

    url += strspn(url, " \t");
    char *version = strpbrk(url, " \t");
    if (!version)
    {
        return BAD_REQUEST;
    }
    *version++ = '\0';
    version += strspn(version, " \t");

    if (strcasecmp(version, "HTTP/1.1") != 0) // 仅支持HTTP/1.1
    {
        return BAD_REQUEST;
    }

    if (strncasecmp(url, "http://", 7) == 0) // 跳过http://
    {
        url += 7;
        url = strchr(url, '/');
    }

    if (!url || url[0] != '/')
    {
        return BAD_REQUEST;
    }

    p_url = url;
    p_check_state = CHECK_STATE_HEADER;
    return NO_REQUEST;
}

// 解析请求头
http_conn::HTTP_CODE http_conn::process_request_header(char *text)
{
    if (text[0] == '\0')
    {
        if (p_content_length != 0)
        {
            p_check_state = CHECK_STATE_CONTENT;
            return NO_REQUEST;
        }
        return GET_REQUEST;
    }
    else if (strncasecmp(text, "Host:", 5) == 0) // 解析Host字段
    {
        text += 5;
        text += strspn(text, " \t");
        p_host = text;
    }
    else if (strncasecmp(text, "Connection:", 11) == 0) // 解析Connection字段
    {
        text += 11;
        text += strspn(text, " \t");
        if (strncasecmp(text, "keep-alive", 10) == 0) // 保持连接
        {
            p_linger = true;
        }
    }
    else if (strncasecmp(text, "Content-Length:", 15) == 0) // 解析Content-Length字段
    {
        text += 15;
        text += strspn(text, " \t");
        p_content_length = atol(text);
    }
    else if (strncasecmp(text, "Accept-Encoding:", 16) == 0) // 解析Accept-Encoding字段
    {
        text += 16;
        text += strspn(text, " \t");
        p_accept_encoding = text;
    }
    else
    {
        // 其他头部字段可以根据需要解析
    }
    return NO_REQUEST;
}

// 解析请求体
http_conn::HTTP_CODE http_conn::process_request_content(char *text)
{
    if (p_read_idx >= (p_content_length + p_checked_idx)) // 请求体读取完毕
    {
        text[p_content_length] = '\0';
        p_content = text;
        return GET_REQUEST;
    }
    return NO_REQUEST;
}

// 解析一行
http_conn::LINE_STATUS http_conn::parse_line()
{
    char temp;
    for (; p_checked_idx < p_read_idx; ++p_checked_idx)
    {
        temp = p_read_buf[p_checked_idx];
        if (temp == '\r')
        {
            if ((p_checked_idx + 1) == p_read_idx)
            {
                return LINE_OPEN;
            }
            else if (p_read_buf[p_checked_idx + 1] == '\n')
            {
                p_read_buf[p_checked_idx++] = '\0';
                p_read_buf[p_checked_idx++] = '\0';
                return LINE_OK;
            }
            return LINE_BAD;
        }
        else if (temp == '\n')
        {
            if (p_checked_idx > 1 && p_read_buf[p_checked_idx - 1] == '\r')
            {
                p_read_buf[p_checked_idx - 1] = '\0';
                p_read_buf[p_checked_idx++] = '\0';
                return LINE_OK;
            }
            return LINE_BAD;
        }
    }
    return LINE_OPEN;
}

// 获取一行
char *http_conn::get_line()
{
    return p_read_buf + p_start_line;
}

// 处理请求,将请求文件映射到内存
http_conn::HTTP_CODE http_conn::do_request()
{
    strcpy(p_real_file, doc_root);
    int len = strlen(doc_root);
    strncpy(p_real_file + len, p_url, FILENAME_LEN - len - 1);
    // 获取m_real_file文件的相关的状态信息，-1失败，0成功
    if (stat(p_real_file, &p_file_stat) < 0)
    {
        return NO_RESOURCE;
    }

    // 判断访问权限
    if (!(p_file_stat.st_mode & S_IROTH))
    {
        return FORBIDDEN_REQUEST;
    }

    // 判断是否是目录
    if (S_ISDIR(p_file_stat.st_mode))
    {
        return BAD_REQUEST;
    }

    // 以只读方式打开文件
    int fd = open(p_real_file, O_RDONLY);
    // 创建内存映射
    p_file_address = (char *)mmap(0, p_file_stat.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);
    return FILE_REQUEST;
}

// 取消映射文件到内存
void http_conn::unmap()
{
    if (p_file_address) // 如果文件已经映射到内存
    {
        munmap(p_file_address, p_file_stat.st_size); // 取消映射
        p_file_address = nullptr;                    // 重置文件映射地址
    }
}

// 往响应中写入内容
bool http_conn::add_response(const char *format, ...)
{
    if (p_write_idx >= WRITE_BUFFER_SIZE)
    { // 检查缓冲区是否已满
        return false;
    }
    va_list arg_list;
    va_start(arg_list, format);                                                                            // 初始化可变参数列表
    int len = vsnprintf(p_write_buf + p_write_idx, WRITE_BUFFER_SIZE - 1 - p_write_idx, format, arg_list); // 格式化字符串并写入缓冲区
    if (len >= (WRITE_BUFFER_SIZE - 1 - p_write_idx))
    { // 检查是否超出缓冲区大小
        return false;
    }
    p_write_idx += len; // 更新缓冲区写入索引
    va_end(arg_list);   // 清理可变参数列表
    return true;
}

// 添加HTTP状态行到响应中
bool http_conn::add_status_line(int status, const char *title)
{
    return add_response("%s %d %s\r\n", "HTTP/1.1", status, title);
}

// 添加HTTP头部到响应中
bool http_conn::add_headers(int content_len)
{
    add_content_length(content_len); // 添加Content-Length头部
    add_content_type();              // 添加Content-Type头部
    add_linger();                    // 添加Connection头部
    add_blank_line();                // 添加一个空行，表示头部结束
}

// 添加Content-Length头部到响应中
bool http_conn::add_content_length(int content_len)
{
    return add_response("Content-Length: %d\r\n", content_len);
}

// 添加Connection头部到响应中
bool http_conn::add_linger()
{
    return add_response("Connection: %s\r\n", (p_linger == true) ? "keep-alive" : "close");
}

// 添加一个空行到响应中，表示头部结束
bool http_conn::add_blank_line()
{
    return add_response("%s", "\r\n");
}

// 添加内容到响应中
bool http_conn::add_content(const char *content)
{
    return add_response("%s", content);
}

// 添加Content-Type头部到响应中
bool http_conn::add_content_type()
{
    return add_response("Content-Type: text/html; charset=UTF-8\r\n");
}

// 根据服务器处理HTTP请求的结果，决定返回给客户端的内容
bool http_conn::process_write(HTTP_CODE ret)
{
    switch (ret)
    {
    case INTERNAL_ERROR: // 内部服务器错误
        add_status_line(500, error_500_title);
        add_headers(strlen(error_500_form));
        if (!add_content(error_500_form))
        {
            return false;
        }
        break;
    case BAD_REQUEST: // 坏请求
        add_status_line(400, error_400_title);
        add_headers(strlen(error_400_form));
        if (!add_content(error_400_form))
        {
            return false;
        }
        break;
    case NO_RESOURCE: // 资源不存在
        add_status_line(404, error_404_title);
        add_headers(strlen(error_404_form));
        if (!add_content(error_404_form))
        {
            return false;
        }
        break;
    case FORBIDDEN_REQUEST: // 禁止访问
        add_status_line(403, error_403_title);
        add_headers(strlen(error_403_form));
        if (!add_content(error_403_form))
        {
            return false;
        }
        break;
    case FILE_REQUEST: // 文件请求
        add_status_line(200, ok_200_title);
        add_headers(p_file_stat.st_size);
        p_iv[0].iov_base = p_write_buf;
        p_iv[0].iov_len = p_write_idx;
        p_iv[1].iov_base = p_file_address;
        p_iv[1].iov_len = p_file_stat.st_size;
        p_iv_count = 2;
        return true;
    default:
        return false;
    }

    p_iv[0].iov_base = p_write_buf;
    p_iv[0].iov_len = p_write_idx;
    p_iv_count = 1;
    return true;
}

// 处理客户请求的入口函数
void http_conn::process()
{
    // 解析HTTP请求
    HTTP_CODE read_ret = process_read();
    if (read_ret == NO_REQUEST)
    {
        modfd(p_epollfd, p_sockfd, EPOLLIN);
        return;
    }

    // 生成响应
    bool write_ret = process_write(read_ret);
    if (!write_ret)
    {
        close_conn();
    }
    modfd(p_epollfd, p_sockfd, EPOLLOUT);
}