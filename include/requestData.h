/***********************************************************
 *@author RedDragon
 *@date 2020/8/26
 *@brief 解析http请求的相关函数和类
 * http状态机
 * while就是为了状态的转移
***********************************************************/

#ifndef WEBSERVER_REQUESTDATA_H
#define WEBSERVER_REQUESTDATA_H
#include "useEpoll.h"
#include "util.h"
#include "timer.h"

#include <opencv/cv.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>

using namespace std;

// 解析http request报文的状态
enum class ParseRequest{
    PARSE_ONELINE,  // 首行请求行
    PARSE_HEADERS,  // 头部字段
    PARSE_BODY,     // 主体数据
    ANALYSIS,       // 分析报文
    FINISH          // 完成
};
// 解析头部字段状态
enum class ParseHeader{
    h_start,// 头部字段开始
    h_key,// 头部字段key
    h_colon,// 冒号
    h_spaces_after_colon,// 冒号后面的空格
    h_value,// 头部字段value
    h_CR,// 回车
    h_LF,// 换行
    h_end_CR,
    h_end_LF
};
// 解析结果(通用, 无论是解析请求行还是头部字段)
enum class ParseResult{
    AGAIN,
    ERROR,
    SUCCESS
};

enum class httpMethod{
    // http1.0
    GET,
    POST,
    HEAD,
    // http1.1新增
    OPTIONS,
    DELETE,
    PUT,
    TRACE,
    PATCH,
    CONNECT
};

// 对EAGAIN这样的错误尝试超过一定的次数就抛弃
const int AGAIN_MAX_TIMES = 200;
// 等待时间，也是keep-alive等待事件
const int EPOLL_WAIT_TIME = 500;

struct requestData
{
private:
    int againTime;          // 表示readn重复尝试的次数
    int clientFd;           // 客户端fd
    int epoll_fd;           // epoll实例
    std::string path;       // 文件夹目录
    std::string content;    // readn()读到的内容(也就是请求报文的所有内容)
    httpMethod method;      // 此次请求的方法
    std::string httpVer;// http版本
    std::string file_name;  // 文件路径
    int now_read_pos;

    ParseRequest state;// 当前解析request报文的状态
    ParseHeader h_state;// headers的状态
    bool isFinish;
    bool keep_alive;
    std::unordered_map<std::string, std::string> headers;// 所有头部字段
    timer itimer;

private:
    // 解析请求行（第一行）
    ParseResult parse_OneLine();
    // 解析头部字段，并保存到headers
    ParseResult parse_Headers();
    // 处理请求
    ParseResult analysisRequest();

public:
    requestData();
    requestData(int _epoll_fd, int _fd, const std::string& _path);
    ~requestData();
    int getClientFd() const{return clientFd;}
    bool isTimeout()const{return itimer.isExpired();}

    void handleRequest();// 解析http请求的 起点
    void handleError(int fd, int err_num, std::string short_msg);
};

#endif //WEBSERVER_REQUESTDATA_H
