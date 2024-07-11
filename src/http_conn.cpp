#include "http_conn.h"

int HttpConnection::epollFd = -1;
int HttpConnection::userCount = 0;

HttpConnection::HttpConnection()
    : sockfd(-1), fileAddress(nullptr), readBuffer(READ_BUFFER_SIZE), writeBuffer(WRITE_BUFFER_SIZE) {}

HttpConnection::~HttpConnection()
{
    if (sockfd != -1)
    {
        close(sockfd);
    }
    if (fileAddress)
    {
        unmap();
    }
}

void HttpConnection::init(int sockfd, const sockaddr_in &addr, EpollMode mode)
{
    this->sockfd = sockfd;
    address = addr;
    int opt = 1;
    setsockopt(this->sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    addFd(epollFd, sockfd, true, mode);
    userCount++;
    initConnection();
}

void HttpConnection::closeConnection()
{
    if (sockfd != -1)
    {
        spdlog::debug("关闭连接: {}", sockfd);
        removeFd(epollFd, sockfd);
        close(sockfd);
        sockfd = -1;
        writeIndex = 0;
        userCount--;
    }
}

void HttpConnection::initConnection()
{
    spdlog::debug("初始化连接参数");
    checkState = CheckState::REQUEST_LINE;
    linger = false;
    method = Method::GET;
    url.clear();
    version.clear();
    contentLength = 0;
    host.clear();
    startLine = 0;
    checkedIndex = 0;
    readIndex = 0;
    writeIndex = 0;
    std::fill(readBuffer.begin(), readBuffer.end(), '\0');
    std::fill(writeBuffer.begin(), writeBuffer.end(), '\0');
    realFile.clear();
}

bool HttpConnection::read()
{
    if (readIndex >= READ_BUFFER_SIZE)
    {
        std::cerr << "Read buffer overflow" << std::endl;
        return false;
    }
    int bytesRead = 0;
    while (true)
    {
        bytesRead = recv(sockfd, readBuffer.data() + readIndex, READ_BUFFER_SIZE - readIndex, 0);
        if (bytesRead == -1)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                break;
            }
            return false;
        }
        else if (bytesRead == 0)
        {
            std::cerr << "Client closed connection" << std::endl;
            return false;
        }
        readIndex += bytesRead;
        if (readIndex >= READ_BUFFER_SIZE)
        {
            std::cerr << "Read buffer overflow" << std::endl;
            return false;
        }
    }
    spdlog::debug("从连接 {} 读到数据: {}", sockfd, readBuffer.data());
    return true;
}

bool HttpConnection::write()
{
    int temp = 0;
    int bytesHaveSent = 0;
    int k = writeIndex;
    int bytesToSend = writeIndex;
    spdlog::debug("开始写操作，待发送字节数: {}", bytesToSend);

    if (bytesToSend == 0)
    {
        spdlog::debug("没有数据需要发送，重置连接状态。");
        modFd(epollFd, sockfd, EPOLLIN);
        initConnection();
        return true;
    }

    while (true)
    {
        temp = writev(sockfd, iv, ivCount);

        if (temp <= -1)
        {
            if (errno == EAGAIN)
            {
                spdlog::debug("写缓冲区满，等待下一次写入机会。");
                modFd(epollFd, sockfd, EPOLLOUT);
                return true;
            }
            spdlog::error("写入错误，关闭连接 {}。错误码: {}", sockfd, errno);
            unmap();
            return false;
        }

        spdlog::debug("写入字节数: {}", temp);

        bytesToSend -= temp;
        bytesHaveSent += temp;


        if (bytesToSend <= 0)
        {
            unmap();
            if (linger)
            {
                spdlog::debug("用户保持连接，重置连接状态。");
                initConnection();
                modFd(epollFd, sockfd, EPOLLIN);
                return true;
            }
            else
            {
                spdlog::debug("短连接，关闭连接。");
                modFd(epollFd, sockfd, EPOLLIN);
                return false;
            }
        }
    }
}

HttpConnection::HttpCode HttpConnection::processRead()
{
    LineStatus lineStatus = LineStatus::OK;
    HttpCode ret = HttpCode::NO_REQUEST;
    std::string text;

    while ((lineStatus = parseLine()) == LineStatus::OK)
    {
        text = getLine();
        startLine = checkedIndex;
        switch (checkState)
        {
        case CheckState::REQUEST_LINE:
        {
            ret = parseRequestLine(text);
            if (ret == HttpCode::BAD_REQUEST)
            {
                return HttpCode::BAD_REQUEST;
            }
            break;
        }
        case CheckState::HEADER:
        {
            ret = parseHeaders(text);
            if (ret == HttpCode::BAD_REQUEST)
            {
                return HttpCode::BAD_REQUEST;
            }
            else if (ret == HttpCode::GET_REQUEST)
            {
                return handleRequest();
            }
            break;
        }
        case CheckState::CONTENT:
        {
            ret = parseContent(text);
            if (ret == HttpCode::GET_REQUEST)
            {
                return handleRequest();
            }
            lineStatus = LineStatus::OPEN;
            break;
        }
        default:
        {
            return HttpCode::INTERNAL_ERROR;
        }
        }
    }

    return HttpCode::NO_REQUEST;
}

HttpConnection::HttpCode HttpConnection::parseRequestLine(std::string &text)
{
    auto urlPos = text.find(' ');
    if (urlPos == std::string::npos)
    {
        return HttpCode::BAD_REQUEST;
    }

    std::string methodStr = text.substr(0, urlPos);
    if (methodStr == "GET")
    {
        method = Method::GET;
    }
    else
    {
        return HttpCode::BAD_REQUEST;
    }

    auto versionPos = text.find(' ', urlPos + 1);
    if (versionPos == std::string::npos)
    {
        return HttpCode::BAD_REQUEST;
    }

    url = text.substr(urlPos + 1, versionPos - urlPos - 1);
    version = text.substr(versionPos + 1);
    if (version != "HTTP/1.1")
    {
        return HttpCode::BAD_REQUEST;
    }

    if (url.find("http://") == 0)
    {
        url = url.substr(7);
        auto urlStartPos = url.find('/');
        if (urlStartPos != std::string::npos)
        {
            url = url.substr(urlStartPos);
        }
    }

    if (url.empty() || url[0] != '/')
    {
        return HttpCode::BAD_REQUEST;
    }

    checkState = CheckState::HEADER;
    return HttpCode::NO_REQUEST;
}

HttpConnection::HttpCode HttpConnection::parseHeaders(std::string &text)
{
    if (text.empty())
    {
        if (contentLength != 0)
        {
            checkState = CheckState::CONTENT;
            return HttpCode::NO_REQUEST;
        }
        return HttpCode::GET_REQUEST;
    }
    else if (text.find("Host:") == 0)
    {
        host = text.substr(5);
        host.erase(0, host.find_first_not_of(" \t"));
    }
    else if (text.find("Connection:") == 0)
    {
        std::string connStr = text.substr(11);
        connStr.erase(0, connStr.find_first_not_of(" \t"));
        if (connStr == "keep-alive")
        {
            linger = true;
        }
    }
    else if (text.find("Content-Length:") == 0)
    {
        std::string lenStr = text.substr(15);
        lenStr.erase(0, lenStr.find_first_not_of(" \t"));
        contentLength = std::stol(lenStr);
    }
    else if (text.find("Accept-Encoding:") == 0)
    {
        acceptEncoding = text.substr(16);
        acceptEncoding.erase(0, acceptEncoding.find_first_not_of(" \t"));
    }
    return HttpCode::NO_REQUEST;
}

HttpConnection::HttpCode HttpConnection::parseContent(std::string &text)
{
    if (readIndex >= (contentLength + checkedIndex))
    {
        content = text.substr(0, contentLength);
        return HttpCode::GET_REQUEST;
    }
    return HttpCode::NO_REQUEST;
}

HttpConnection::LineStatus HttpConnection::parseLine()
{
    char temp;
    for (; checkedIndex < readIndex; ++checkedIndex)
    {
        temp = readBuffer[checkedIndex];
        if (temp == '\r')
        {
            if ((checkedIndex + 1) == readIndex)
            {
                return LineStatus::OPEN;
            }
            else if (readBuffer[checkedIndex + 1] == '\n')
            {
                readBuffer[checkedIndex++] = '\0';
                readBuffer[checkedIndex++] = '\0';
                return LineStatus::OK;
            }
            return LineStatus::BAD;
        }
        else if (temp == '\n')
        {
            if (checkedIndex > 1 && readBuffer[checkedIndex - 1] == '\r')
            {
                readBuffer[checkedIndex - 1] = '\0';
                readBuffer[checkedIndex++] = '\0';
                return LineStatus::OK;
            }
            return LineStatus::BAD;
        }
    }
    return LineStatus::OPEN;
}

std::string HttpConnection::getLine()
{
    return std::string(readBuffer.data() + startLine);
}

HttpConnection::HttpCode HttpConnection::handleRequest()
{
    realFile = docRoot + url;
    if (stat(realFile.c_str(), &fileStat) < 0)
    {
        return HttpCode::NO_RESOURCE;
    }

    if (!(fileStat.st_mode & S_IROTH))
    {
        return HttpCode::FORBIDDEN_REQUEST;
    }

    if (S_ISDIR(fileStat.st_mode))
    {
        return HttpCode::BAD_REQUEST;
    }

    int fd = open(realFile.c_str(), O_RDONLY);
    fileAddress = static_cast<char *>(mmap(0, fileStat.st_size, PROT_READ, MAP_PRIVATE, fd, 0));
    close(fd);
    return HttpCode::FILE_REQUEST;
}

void HttpConnection::unmap()
{
    if (fileAddress)
    {
        munmap(fileAddress, fileStat.st_size);
        fileAddress = nullptr;
    }
}

bool HttpConnection::addResponse(const char *format, ...)
{
    if (writeIndex >= WRITE_BUFFER_SIZE)
    {
        return false;
    }
    va_list argList;
    va_start(argList, format);
    int len = vsnprintf(writeBuffer.data() + writeIndex, WRITE_BUFFER_SIZE - 1 - writeIndex, format, argList);
    if (len >= (WRITE_BUFFER_SIZE - 1 - writeIndex))
    {
        return false;
    }
    writeIndex += len;
    int i = this->writeIndex;
    va_end(argList);
    return true;
}

bool HttpConnection::addStatusLine(int status, const std::string &title)
{
    return addResponse("%s %d %s\r\n", "HTTP/1.1", status, title.c_str());
}

bool HttpConnection::addHeaders(int contentLength)
{
    addContentLength(contentLength);
    addContentType();
    addLinger();
    addBlankLine();
    return true;
}

bool HttpConnection::addContentLength(int contentLength)
{
    return addResponse("Content-Length: %d\r\n", contentLength);
}

bool HttpConnection::addLinger()
{
    return addResponse("Connection: %s\r\n", linger ? "keep-alive" : "close");
}

bool HttpConnection::addBlankLine()
{
    return addResponse("%s", "\r\n");
}

bool HttpConnection::addContent(const std::string &content)
{
    return addResponse("%s", content.c_str());
}

bool HttpConnection::addContentType()
{
    return addResponse("Content-Type: text/html; charset=UTF-8\r\n");
}

bool HttpConnection::processWrite(HttpCode ret)
{
    spdlog::debug("处理响应报文");
    switch (ret)
    {
    case HttpCode::INTERNAL_ERROR:
        addStatusLine(500, error500Title);
        addHeaders(error500Form.size());
        if (!addContent(error500Form))
        {
            return false;
        }
        break;
    case HttpCode::BAD_REQUEST:
        addStatusLine(400, error400Title);
        addHeaders(error400Form.size());
        if (!addContent(error400Form))
        {
            return false;
        }
        break;
    case HttpCode::NO_RESOURCE:
        addStatusLine(404, error404Title);
        addHeaders(error404Form.size());
        if (!addContent(error404Form))
        {
            return false;
        }
        break;
    case HttpCode::FORBIDDEN_REQUEST:
        addStatusLine(403, error403Title);
        addHeaders(error403Form.size());
        if (!addContent(error403Form))
        {
            return false;
        }
        break;
    case HttpCode::FILE_REQUEST:
        addStatusLine(200, ok200Title);
        addHeaders(fileStat.st_size);
        iv[0].iov_base = writeBuffer.data();
        iv[0].iov_len = writeIndex;
        iv[1].iov_base = fileAddress;
        iv[1].iov_len = fileStat.st_size;
        ivCount = 2;
        spdlog::debug("添加响应内容成功");
        return true;
    default:
        return false;
    }

    iv[0].iov_base = writeBuffer.data();
    iv[0].iov_len = writeIndex;
    ivCount = 1;
    return true;
}

void HttpConnection::process()
{
    HttpCode readRet = processRead();
    if (readRet == HttpCode::NO_REQUEST)
    {
        modFd(epollFd, sockfd, EPOLLIN);
        return;
    }

    bool writeRet = processWrite(readRet);
    if (!writeRet)
    {
        closeConnection();
    }
    modFd(epollFd, sockfd, EPOLLOUT);
}

void setNonBlocking(int fd)
{
    int oldOption = fcntl(fd, F_GETFL);
    int newOption = oldOption | O_NONBLOCK;
    fcntl(fd, F_SETFL, newOption);
}

void addFd(int epollfd, int fd, bool oneShot, EpollMode mode)
{
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLRDHUP;
    if (mode == EpollMode::ET)
    {
        event.events |= EPOLLET;
    }
    if (oneShot)
    {
        event.events |= EPOLLONESHOT;
    }
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
    setNonBlocking(fd);
}

void removeFd(int epollfd, int fd)
{
    epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, 0);
    close(fd);
}

void modFd(int epollfd, int fd, int ev)
{
    epoll_event event;
    event.data.fd = fd;
    event.events = ev | EPOLLRDHUP | EPOLLONESHOT;
    spdlog::debug("将连接 {} 的事件修改为: {}", fd, static_cast<uint32_t>(event.events));
    if (epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &event) == -1)
    {
        spdlog::error("修改事件失败: {},epollfd: {}", strerror(errno), epollfd);
    }
    else
    {
        spdlog::info("成功修改连接 {} 的事件", fd);
    }
}
