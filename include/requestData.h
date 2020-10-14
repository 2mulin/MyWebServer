/***********************************************************
 *@author RedDragon
 *@date 2020/8/26
 *@brief 解析http请求的相关函数和类
***********************************************************/

#ifndef WEBSERVER_REQUESTDATA_H
#define WEBSERVER_REQUESTDATA_H

#include <string>
#include <unordered_map>
#include <queue>
#include <iostream>
#include <unistd.h>
#include <cstdio>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/time.h>

#include "useEpoll.h"
#include "util.h"

#include <opencv/cv.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>

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

// http请求方法
const int METHOD_POST = 1;
const int METHOD_GET = 2;

// http版本
const int HTTP_10 = 1;
const int HTTP_11 = 2;

// 等待时间，也是keep-alive等待事件
const int EPOLL_WAIT_TIME = 500;

using namespace std;

// 单例模式，根据后缀判断get请求的文件类型
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
    h_start = 0,// 头部字段开始
    h_key,// 头部字段key
    h_colon,// 冒号
    h_spaces_after_colon,// 冒号后面的空格
    h_value,// 头部字段value
    h_CR,// 回车
    h_LF,// 换行
    // 空行，头部结束
    h_end_CR,
    h_end_LF
};

// 前置声明
struct mytimer;

//
struct requestData
{
private:
    int againTime;
    int fd;// 发送这个request的客户端fd
    int epoll_fd;// epoll实例
    std::string path;// 文件夹目录
    std::string content;// readn()读到的请求报文全部内容
    int method;
    int HttpVersion;
    std::string file_name;// 文件路径
    int now_read_pos;
    int state;// 解析报文的状态
    int h_state;// headers的状态
    bool isFinish;
    bool keep_alive;
    std::unordered_map<std::string, std::string> headers;// 请求报文头部字段
    mytimer *timer;

private:
    // 解析请求行（第一行）
    int parse_URI();

    // 解析头部字段，并保存到headers
    int parse_Headers();

    int analysisRequest();

public:
    requestData();

    requestData(int _epoll_fd, int _fd, const string& _path);

    ~requestData();

    // 添加计时器
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

    // 分离计时器
    void separateTimer();

    void handleRequest();

    void handleError(int fd, int err_num, std::string short_msg);
};

// 计时器，主要负责
struct mytimer
{
    bool deleted;//
    size_t expired_time;// 到期时间
    requestData *request_data;

    mytimer(requestData *_request_data, int timeout);

    ~mytimer();

    // 更新（加长）expired_time
    void update(int timeout);

    // 判断当前是否过期
    bool isValid();

    void clearReq();

    void setDeleted()
    {
        deleted = true;
    }

    // 包含的requestData对象是否被删除
    bool isDeleted() const
    {
        return deleted;
    }

    // 得到过期时间
    size_t getExptime() const
    {
        return expired_time;
    }

//    // 由于priority_queue存放的指针，重载operator<没有作用
//    bool operator<(const mytimer& b) const
//    {
//        return this->getExptime() < b.getExptime();
//    }
};

// 用于构建优先队列
struct timerCmp
{
    bool operator()(const mytimer *a, const mytimer *b) const
    {
        return a->getExptime() > b->getExptime();
    }
};

#endif //WEBSERVER_REQUESTDATA_H
