/***********************************************************
 *@author RedDragon
 *@date 2020/8/26
 *@brief 这个文件的代码有点看不懂，有些地方很奇怪，
***********************************************************/

#include "requestData.h"

// 静态变量初始化
unordered_map<string, string> MimeType::mime;
pthread_mutex_t MimeType::lock = PTHREAD_MUTEX_INITIALIZER;

// 全局变量
pthread_mutex_t qlock = PTHREAD_MUTEX_INITIALIZER;
priority_queue<mytimer *, deque<mytimer *>, timeCmp> myTimequeue;


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
    struct epoll_event ev;
    // 读请求，边缘触发，一次通知
    ev.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
    ev.data.ptr = (void*)this;
    // 从epoll实例中的兴趣列表中删除fd
    epoll_ctl(epoll_fd,EPOLL_CTL_DEL,fd,&ev);
    if(timer != nullptr)
    {
        timer->clearReq();
        timer = nullptr;
    }
    close(fd);
}

// 初始化timer
void requestData::addTimer(mytimer *mtimer)
{
    if(timer == nullptr)
        timer = mtimer;
}

// 重置
void requestData::reset()
{
    againTime = 0;
    content.clear();
    file_name.clear();
    path.clear();
    now_read_pos = 0;
    state = STATE_PARSE_URI;
    h_state = h_start;
    headers.clear();
    keep_alive = false;
}

// 分离计时器
void requestData::separateTimer()
{
    if(timer)
    {
        timer->clearReq();
        timer = nullptr;
    }
}

// 处理请求
void requestData::handleRequest()
{
    char buff[MAX_BUFF];
    bool isError = false;
    while(true)
    {
        int read_num = readn(fd, buff, MAX_BUFF);
        if(read_num < 0)
        {
            perror("1");
            isError = true;
            break;
        }
        else if(read_num == 0)
        {
            // 有请求但是读不到数据，可能是Request Aborted
            perror("read_num == 0");
            if(errno == EAGAIN)
            {
                if(againTime > AGAIN_MAX_TIMES)
                    isError = true;
                else
                    againTime++;
            }
            else if(errno != 0)
                isError = true;
            break;
        }
        string now_read(buff,buff+read_num);
        content += now_read;

        if(state == STATE_PARSE_URI)
        {// 第一个状态,解析URI
            int flag = this->parse_URI();// 去解析URI
            if(flag == PARSE_URI_AGAIN)
                break;
            else if(flag == PARSE_URI_ERROR)
            {
                perror("2");
                isError = true;
                break;
            }
        }

        if(state == STATE_PARSE_HEADERS)
        {// 第二个状态，解析头部
            int flag = this->parse_Headers();
            if(flag == PARSE_HEADER_AGAIN)
                break;
            else if(flag == PARSE_HEADER_ERROR)
            {
                perror("3");
                isError = true;
                break;
            }
            // POST需要body的信息，GET不需要
            if(method == METHOD_POST)
                state = STATE_RECV_BODY;
            else
                state = STATE_ANALYSIS;
        }

        if(state == STATE_RECV_BODY)
        {// POST需要读取body的状态
            int content_length = -1;// 头部字段Content_length的值
            if(headers.find("Content_length") != headers.end())
                content_length = stoi(headers["Content_length"]);
            else
            {
                isError = true;
                break;
            }
            if(content.size() < content_length)
                continue;
            state = STATE_ANALYSIS;
        }

        if(state == STATE_ANALYSIS)
        {
            int flag = this->analysisRequest();
            if(flag < 0)
            {
                isError = true;
                break;
            }
            else if(flag == ANALYSIS_SUCCESS)
            {
                state = STATE_FINISH;
                break;
            }
            else
            {
                isError = true;
                break;
            }
        }

        if(isError)
        {
            delete this;
            return;
        }

        // 加入epoll继续
        if(state == STATE_FINISH)
        {
            if(keep_alive)
            {
                printf("OK\n");
                this->reset();
            }
            else
            {
                delete this;
                return;
            }
        }

        // 一定要先加时间信息，否则可能会出现刚加进去，下个in触发来了，然后分离失败后，又加入队列，
        // 最后超时被删，然后正在线程中进行的任务出错，double free错误。
        // 新增时间信息
        pthread_mutex_lock(&qlock);
        mytimer *mtimer = new mytimer(this,500);
        timer = mtimer;
        myTimequeue.push(mtimer);
        pthread_mutex_unlock(&qlock);

        uint32_t events = EPOLLIN | EPOLLET | EPOLLONESHOT;
        int ret = epoll_mod(epoll_fd, fd, static_cast<void*>(this),events);
        if(ret < 0)
        {
            delete this;
            return;
        }
    }
}
