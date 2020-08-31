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
    int epoll_fd;
    std::string content;
    int method;
    int HTTPversion;
    std::string fileName;
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

    void addTimer(mytimer *timer);

    void reset();

    void separateTimer();

    int getFd();

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
