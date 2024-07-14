#ifndef HTTPCONNECTION_H
#define HTTPCONNECTION_H

#include <sys/epoll.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <string>
#include <cstring>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdarg.h>
#include <sys/uio.h>
#include <vector>
#include <memory>
#include <spdlog/spdlog.h>
#include "chatAPI.h"
#include "json.hpp"
constexpr int READ_BUFFER_SIZE = 4096;
constexpr int WRITE_BUFFER_SIZE = 4096;
constexpr int FILENAME_LEN = 200;
constexpr const char *WEB_ROOT = "../resources";

enum class EpollMode
{
    LT,
    ET
};

class HttpConnection
{
public:
    enum class Method
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
    enum class CheckState
    {
        REQUEST_LINE = 0,
        HEADER,
        CONTENT
    };
    enum class HttpCode
    {
        NO_REQUEST,
        GET_REQUEST,
        BAD_REQUEST,
        NO_RESOURCE,
        FORBIDDEN_REQUEST,
        FILE_REQUEST,
        INTERNAL_ERROR,
        CLOSED_CONNECTION,
        AI_RESPONSE,
        OPTIONS_RESPONSE
    };
    enum class LineStatus
    {
        OK = 0,
        BAD,
        OPEN
    };

    HttpConnection();
    ~HttpConnection();

    void process();
    void init(int sockfd, const sockaddr_in &addr, EpollMode mode);
    void closeConnection();
    bool read();
    bool write();
    std::string generateJsonResponse(const std::string &content);
    int writeIndex;

    static int epollFd;
    static int userCount;

private:
    void initConnection();
    std::string getLine();
    HttpCode processRead();
    HttpCode parseRequestLine(std::string &text);
    HttpCode parseHeaders(std::string &text);
    HttpCode parseContent(std::string &text);
    LineStatus parseLine();
    HttpCode handleRequest();

    void unmap();
    bool processWrite(HttpCode ret);
    bool addResponse(const char *format, ...);
    bool addContent(const std::string &content);
    bool addContentTypeHTML();
    bool addContentTypeJSON();
    bool addStatusLine(int status, const std::string &title);
    bool addHeaders(int contentLength,HttpCode ret);
    bool addContentLength(int contentLength);
    bool addLinger();
    bool addBlankLine();
    bool addCorsHeaders();

    Method method;
    CheckState checkState;
    sockaddr_in address;
    int sockfd;
    std::vector<char> readBuffer;
    int readIndex;
    int checkedIndex;
    int startLine;
    std::string url;
    std::string version;
    std::string host;
    bool linger;
    int contentLength;
    std::string content;
    std::string acceptEncoding;
    std::string realFile;
    std::vector<char> writeBuffer;
    char *fileAddress;
    struct stat fileStat;
    struct iovec iv[2];
    int ivCount;
    std::string responseText;

    const std::string ok200Title = "OK";
    const std::string error400Title = "Bad Request";
    const std::string error400Form = "Your request has bad syntax or is inherently impossible to satisfy.\n";
    const std::string error403Title = "Forbidden";
    const std::string error403Form = "You do not have permission to get file from this server.\n";
    const std::string error404Title = "Not Found";
    const std::string error404Form = "The requested file was not found on this server.\n";
    const std::string error500Title = "Internal Error";
    const std::string error500Form = "There was an unusual problem serving the requested file.\n";
    const std::string docRoot = WEB_ROOT;
};

void setNonBlocking(int fd);
void addFd(int epollfd, int fd, bool oneShot, EpollMode mode);
void removeFd(int epollfd, int fd);
void modFd(int epollfd, int fd, int ev);

#endif // HTTPCONNECTION_H
