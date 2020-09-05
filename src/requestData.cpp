/***********************************************************
 *@author RedDragon
 *@date 2020/8/26
 *@brief
 * 在requestData::handleRequest()内部使用readn接收http请求，放在
 * 字符串content中。
 * string.find()判断找到没有下面用的是<0,可以改成npos
***********************************************************/

#include <cstring>
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


// requestData类的实现

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

int requestData::parse_URI()
{
    // content的引用(干嘛不直接使用content？？？？)
    string &str = content;
    // 找到第一行回车换行的位置
    int pos = str.find('\r', now_read_pos);
    if(pos < 0)
        return PARSE_URI_AGAIN;
    // str去掉了请求行，现在只有request_line是请求行
    string request_line = str.substr(0 ,pos);
    if(str.size() > pos + 1)
        str = str.substr(pos + 1);
    else
        str.clear();
    // http method,这个判断http方法的代码，也挺秀，干嘛写成这样？？？？
    pos = request_line.find("GET");
    if(pos < 0)
    {
        pos = request_line.find("POST");
        if(pos < 0)
            return PARSE_URI_ERROR;
        else
            method = METHOD_GET;
    }
    else
    {
        method = METHOD_POST;
    }
    // filename
    pos = request_line.find("/", pos);
    if(pos < 0)
    {
        return PARSE_URI_ERROR;
    }
    else
    {
        int _pos = request_line.find(' ', pos);
        if(_pos < 0)
            return PARSE_URI_ERROR;
        else
        {
            if(_pos - pos > 1)
            {
                file_name = request_line.substr(pos + 1, _pos - pos - 1);
                int p = file_name.find('?');
                if(p > 0)
                    file_name = file_name.substr(0, p);
            }
            else
                file_name = "index.html";
        }
        pos = _pos;
    }
    // HTTP版本号
    pos = request_line.find('/',pos);
    if(pos < 0)
    {
        return PARSE_URI_ERROR;
    }
    else
    {
        if(request_line.size() - pos <= 3)
            return PARSE_URI_ERROR;
        else
        {
            string ver = request_line.substr(pos + 1, 3);
            if(ver == "1.0")
                HTTPversion = HTTP_10;
            else if(ver == "1.1")
                HTTPversion - HTTP_11;
            else
                return PARSE_URI_ERROR;
        }
    }
    state = STATE_PARSE_HEADERS;
    return PARSE_URI_SUCCESS;
}

int requestData::parse_Headers()
{
    // content的引用(干嘛不直接使用content？？？？)
    string &str = content;
    bool notfinsh = true;

    // 这里是要读出所有的头部字段，一个一个字节读,伴随状态改变，以此判断读到哪了
    int key_start = -1, key_end = -1, value_start = -1, value_end = -1;
    int now_read_line_begin = 0;
    for(std::size_t i = 0; i < str.size(); i++)
    {
        switch (h_state)
        {
            case h_start:
            {
                if(str[i] == '\n' || str[i] == '\r')
                    break;
                h_state = h_key;
                key_start = i;
                now_read_line_begin = i;
                break;
            }
            case h_key:
            {
                if(str[i] == ':')
                {// key读取结束
                    key_end = i;
                    if(key_end - key_start <= 0)
                        return PARSE_HEADER_ERROR;
                    h_state = h_colon;
                }
                else if (str[i] == '\n' || str[i] == '\r')
                    return PARSE_HEADER_ERROR;
                break;
            }
            case h_colon:
            {
                if(str[i] == ' ')
                    h_state = h_spaces_after_colon;
                else
                    return PARSE_HEADER_ERROR;
                break;
            }
            case h_value:
            {
                if(str[i] == '\r')
                {
                    h_state = h_CR;
                    value_end = i;
                    if(value_end - value_start <= 0)
                        return PARSE_HEADER_ERROR;// value为空
                }
                else if(i - value_start >= 255)
                    return PARSE_HEADER_ERROR;
                break;
            }
            case h_CR:
            {
                if(str[i] == '\n')
                {// 一行结束，保存key和value
                    h_state = h_LF;
                    // key
                    string key(str.begin() + key_start,str.begin() + key_start + key_end);
                    // value
                    string value(str.begin() + value_start, str.begin() + value_start + value_end);
                    headers[key] = value;
                    now_read_line_begin = i;
                }
                else
                    return PARSE_HEADER_ERROR;
                break;
            }
            case h_LF:
            {
                if (str[i] == '\r')
                    h_state = h_end_CR;
                else
                {// 读取下一个头部字段
                    key_start = i;
                    h_state = h_key;
                }
                break;
            }
            case h_end_CR:
            {
                if(str[i] == '\n')
                    h_state = h_end_LF;
                else
                    return PARSE_HEADER_ERROR;
                break;
            }
            case h_end_LF:
            {// 结束
                notfinsh = true;
                key_start = i;
                now_read_line_begin = i;
                break;
            }
        }
    }
    if(h_state == h_end_LF)
    {// 正常结束
        str = str.substr(now_read_line_begin);
        return PARSE_HEADER_SUCCESS;
    }
    str = str.substr(now_read_line_begin);
    return PARSE_HEADER_AGAIN;
}

int requestData::analysisRequest()
{
    if(method == METHOD_POST)
    {
        char header[MAX_BUFF];// 即将发送响应报文头部
        sprintf(header, "HTTP1.1 %d %s\r\n", 200 , "OK");
        if(headers.find("Connection") != headers.end() && headers["Connection"] == "keep-alive")
        {
            keep_alive = true;
            sprintf(header, "%sConnection: keep-alive\r\n", header);
            // Keep-Alive 是一个通用消息头，允许消息发送者暗示连接的状态，还可以用来设置超时时长和最大请求数。
            sprintf(header, "%sKeep-Alive: timeout=%d\r\n",header,EPOLL_WAIT_TIME);
        }

        char *send_content = "I have receiced this.";
        // %zu的z表示length（size_t尺寸大小），u表示无符号整数
        sprintf(header, "%sContent-length: %zu\r\n",header, strlen(send_content));

        // 头部完成，添加空行
        sprintf(header, "%s\r\n",header);
        // 发送request头部
        std::size_t send_len = (std::size_t)writen(fd,header,strlen(header));
        if(send_len != strlen(header))
        {
            perror("Send header failed");
            return ANALYSIS_ERROR;
        }
        // 发送request内容
        send_len = (std::size_t)writen(fd, send_content, strlen(send_content));
        if(send_len != strlen(send_content))
        {
            perror("Send content failed");
            return ANALYSIS_ERROR;
        }
        cout << "content size == " << content.size() << endl;

        // 这里是opencv的代码，不知道为什么对请求报文的body调用。
        vector<char> data(content.begin(),content.end());
        cv::Mat test = cv::imdecode(data,CV_LOAD_IMAGE_ANYDEPTH | CV_LOAD_IMAGE_ANYCOLOR);
        cv::imwrite("receive.bmp", test);
        return ANALYSIS_SUCCESS;
    }
    else if(method == METHOD_GET)
    {
        char header[MAX_BUFF];
        sprintf(header,"HTTP1.1 %d %s\r\n", 200, "OK");
        if(headers.find("Connection") != headers.end() && headers["Connection"] == "keep_alive")
        {
            keep_alive = true;
            sprintf(header,"%sConnection: keep-alive\r\n", header);
            sprintf(header,"%sKeep-Alive: timeout=%d\r\n",header, EPOLL_WAIT_TIME);
        }
        int dot_pos = file_name.find('.');
        const char* fileType;
        if(dot_pos < 0)
            fileType = MimeType::getMime("default").c_str();
        else
            fileType = MimeType::getMime(file_name.substr(dot_pos)).c_str();
        struct stat sbuf;
        if(stat(file_name.c_str(), &sbuf) < 0)
        {// 判断请求文件是否存在
            handleError(fd, 404, "404 Not Found!");
            return ANALYSIS_ERROR;
        }

        sprintf(header, "%sContent-type: %s\r\n", header, fileType);
        sprintf(header, "%sContent-type: %ld\r\n", header, sbuf.st_size);// st_size是文件大小
        sprintf(header, "%s\r\n", header);
        size_t send_len = (size_t)writen(fd, header, strlen(header));
        if(send_len != strlen(header))
        {
            perror("Send header failed");
            return ANALYSIS_ERROR;
        }
        // 打开文件，把内容映射到内存
        int src_fd = open(file_name.c_str(),O_RDONLY);
        char *src_addr = static_cast<char*>(mmap(nullptr, sbuf.st_size, PROT_READ, MAP_PRIVATE, src_fd, 0));
        close(src_fd);
        // 发送文件内容
        send_len = writen(fd, src_addr,sbuf.st_size);
        if(send_len != sbuf.st_size)
        {
            perror("Send file Failed");
            return ANALYSIS_ERROR;
        }
        munmap(src_addr, sbuf.st_size);
        return ANALYSIS_SUCCESS;
    }
    else
        return ANALYSIS_ERROR;
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
        int read_num = readn(fd, buff, MAX_BUFF);// 读数据

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
        content += now_read;// content放请求报文所有内容

        if(state == STATE_PARSE_URI)
        {// 第一个状态,解析URI
            int flag = this->parse_URI();// 去解析URI(请求行)
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
            int flag = this->parse_Headers();// 去解析header
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

        // 还没有add，怎么就mod了，add在哪？
        uint32_t events = EPOLLIN | EPOLLET | EPOLLONESHOT;
        int ret = epoll_mod(epoll_fd, fd, static_cast<void*>(this),events);
        if(ret < 0)
        {
            delete this;
            return;
        }
    }
}

void requestData::handleError(int fd, int err_num, std::string short_msg)
{// 我觉得这函数写的贼垃圾，我可以写的更好
    short_msg = " " + short_msg;
    char send_buf[MAX_BUFF];
    string body_buff =
            "<html>"
            "<title>TKeed Error</title>"
            "<body bgcolor = \"ffffff\">";
    body_buff += to_string(err_num) + short_msg;
    body_buff += "<hr><em> LinYa's Web Server</em>\n</body></html>";

    string header_buff;
    header_buff += "HTTP/1.1 " + to_string(err_num) + short_msg + "\r\n";
    header_buff += "Content-type: text/html\r\n";
    header_buff += "Connection: close\r\n";
    header_buff += "Content-length: " + to_string(body_buff.size()) + "\r\n";
    header_buff += "\r\n";

    sprintf(send_buf, "%s", header_buff.c_str());
    writen(fd, send_buf, strlen(send_buf));// 写入header
    sprintf(send_buf, "%s", body_buff.c_str());
    writen(fd, send_buf,strlen(send_buf));// 写入body
}

// mytimer类的实现

mytimer::mytimer(requestData *_request_data, int timeout)
    :deleted(false), request_data(_request_data)
{
    struct timeval now;
    gettimeofday(&now, nullptr);// 返回日历时间到now中
    // expired_time 表示的毫秒
    expired_time = ((now.tv_sec * 1000) + (now.tv_usec / 1000)) + timeout;
}

mytimer::~mytimer()
{
    cout << "~mytimer()" << endl;
    if(request_data != nullptr)
    {
        cout << "request_data=" << request_data << endl;
        delete request_data;
        request_data = nullptr;
    }
}

void mytimer::update(int timeout)
{
    struct timeval now;
    gettimeofday(&now, nullptr);
    expired_time = ((now.tv_sec * 1000) + (now.tv_usec / 1000)) + timeout;
}

bool mytimer::isValid()
{
    struct timeval now;
    gettimeofday(&now, nullptr);
    size_t temp = (now.tv_sec * 1000 + now.tv_usec / 1000);
    if(temp < expired_time)
        return true;
        else
    {
        this->setDeleted();
        return false;
    }
}

void mytimer::clearReq()
{
    delete request_data;
    this->setDeleted();
}
