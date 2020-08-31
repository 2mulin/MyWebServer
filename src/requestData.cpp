/***********************************************************
 *@author RedDragon
 *@date 2020/8/26
 *@brief 
***********************************************************/

#include "requestData.h"

// 解析状态值
const int STATE_PARSE_URI = 1;
const int STATE_PARSE_HEADERS = 2;
const int STATE_RECV_BODY = 3;
const int STATE_ANALYSIS = 4;
const int STATE_FINISH = 5;

// 静态变量初始化
unordered_map<string, string> MimeType::mime;
pthread_mutex_t MimeType::lock = PTHREAD_MUTEX_INITIALIZER;

// 根据文件后缀确定要发送的数据类型
std::string MimeType::getMime(const std::string &suffix)
{
    if (mime.empty())
    {
        pthread_mutex_lock(&lock);
        if (mime.empty())
        {
            mime[".html"] = "text/html";
            mime[".avi"] = "video/x-msvideo";
            mime[".bmp"] = "image/bmp";
            mime[".c"] = "text/plain";
            mime[".doc"] = "application/msword";
            mime[".gif"] = "image/gif";
            mime[".gz"] = "application/x-gzip";
            mime[".htm"] = "text/html";
            mime[".ico"] = "application/x-ico";
            mime[".jpg"] = "image/jpeg";
            mime[".png"] = "image/png";
            mime[".txt"] = "text/plain";
            mime[".mp3"] = "audio/mp3";
            mime["default"] = "text/html";
        }
        pthread_mutex_lock(&lock);
    }
    if (mime.find(suffix) != mime.end())
        return mime[suffix];
    else
        return mime["default"];
}

priority_queue<mytimer *, deque<mytimer *>, timeCmp> myTimequeue;

requestData::requestData()
        : againTime(0),
          now_read_pos(0),
          state(STATE_PARSE_URI),
          h_state(HeaderState::h_start),
          keep_alive(false),
          timer(nullptr)
{
    cout << "request constructed !" << endl;
}

requestData::requestData(int _epoll_fd, int _fd, std::string _path)
        : againTime(0),
          path(_path),
          fd(_fd),
          epoll_fd(_epoll_fd),
          now_read_pos(0),
          state(STATE_PARSE_URI),
          h_state(HeaderState::h_start),
          keep_alive(false),
          timer(nullptr)
{}

requestData::~requestData()
{
    cout << "~requestData()" << endl;
}