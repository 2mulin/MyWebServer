/***********************************************************
 *@author RedDragon
 *@date 2020/8/26
 *@brief 
***********************************************************/

#ifndef WEBSERVER_REQUESTDATA_H
#define WEBSERVER_REQUESTDATA_H

#include <string>
#include <unordered_map>
#include <queue>
#include <iostream>
#include <unistd.h>
#include <cstdio>
#include <useEpoll.h>
#include "util.h"

// 解析request的状态
const int STATE_PARSE_URI = 1;
const int STATE_PARSE_HEADERS = 2;
const int STATE_RECV_BODY = 3;
const int STATE_ANALYSIS = 4;
const int STATE_FINISH = 5;

const int MAX_BUFF = 4096;

// 有请求出现但是读不到数据,可能是Request Aborted,
// 或者来自网络的数据没有达到等原因,
// 对这样的请求尝试超过一定的次数就抛弃
const int AGAIN_MAX_TIMES = 200;

// 解析request URI的状态
const int PARSE_URI_AGAIN = -1;
const int PARSE_URI_ERROR = -2;
const int PARSE_URI_SUCCESS = 0;

// 解析request header的状态
const int PARSE_HEADER_AGAIN = -1;
const int PARSE_HEADER_ERROR = -2;
const int PARSE_HEADER_SUCCESS = 0;

const int ANALYSIS_ERROR = -2;
const int ANALYSIS_SUCCESS = 0;

const int METHOD_POST = 1;
const int METHOD_GET = 2;
const int HTTP_10 = 1;
const int HTTP_11 = 2;

const int EPOLL_WAIT_TIME = 500;

using namespace std;

// 用到了单例模式
class MimeType
{
private:
    static pthread_mutex_t lock;
    static std::unordered_map<std::string, std::string> mime;

    // 私有化构造函数
    MimeType() = default;

    MimeType(const MimeType &m);

public:
    static std::string getMime(const std::string &suffix);
};

enum HeaderState
{
    h_start = 0,
    h_key,
    h_colon,
    h_spaces_after_colon,
    h_value,
    h_CR,
    h_LF,
    h_end_CR,
    h_end_LF
};

// 前置声明
struct mytimer;

struct requestData
{
private:
    int againTime;
    std::string path;
    int fd;
    int epoll_fd;// epoll实例
    std::string content;// http请求的主体部分
    int method;
    int HTTPversion;
    std::string file_name;
    int now_read_pos;
    int state;
    int h_state;
    bool isFinish;
    bool keep_alive;
    std::unordered_map<std::string, std::string> headers;
    mytimer *timer;

private:
    int parse_URI();

    int parse_Headers();

    int analysisRequest();

public:
    requestData();

    requestData(int _epoll_fd, int _fd, std::string _path);

    ~requestData();

    void addTimer(mytimer *mtimer);

    int getFd() const
    {
        return fd;
    }

    void setFd(int _fd)
    {
        fd = _fd;
    }

    void reset();

    void separateTimer();

    void handleRequest();

    void handleError(int fd, int err_num, std::string short_msg);
};

struct mytimer
{
    bool deleted;
    size_t expired_time;
    requestData *request_data;

    mytimer(requestData *_request_data, int timeout);

    ~mytimer();

    void update(int timeout);

    bool isValid();

    void clearReq();

    void setDeleted();

    bool isDeleted() const;

    size_t getExptime() const;// 到期时间
};

struct timeCmp
{
    bool operator()(const mytimer *a, const mytimer *b) const;
};

#endif //WEBSERVER_REQUESTDATA_H
