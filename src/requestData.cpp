/***********************************************************
 *@author RedDragon
 *@date 2020/8/26
 *@brief requestData的实现
 * 支持GET, POST
***********************************************************/
#include "requestData.h"

requestData::requestData()
    : requestData(-1, -1, "/")
{}

// 初始化列表的顺序必须和class的变量申明顺序一致
requestData::requestData(int cfd, int _epoll_fd, const string& _path)
        : againTime(0),clientFd(cfd),
          epoll_fd(_epoll_fd),path(_path),
          now_read_pos(0),state(ParseRequest::PARSE_ONELINE),
          h_state(ParseHeader::h_start),keep_alive(false)
{
    struct epoll_event ev;
    // EPOLLONESHOT效果是fd只会触发一次, 是为了保证每个client_fd同时只会被一个线程处理
    ev.events = EPOLLIN | EPOLLET | EPOLLONESHOT | EPOLLRDHUP;
    // 指定requestData指针而不是fd的原因是后面epoll_wait返回时,我需要得到fd对应的requestData, 所以直接一步到位
    ev.data.ptr = (void*)this;
    epoll_add(epoll_fd, clientFd, &ev);
}

requestData::~requestData()
{
    struct epoll_event ev;
    ev.data.ptr = this;
    ev.events = EPOLLIN | EPOLLET | EPOLLONESHOT | EPOLLRDHUP;
    epoll_del(epoll_fd, clientFd, &ev);
    close(clientFd);
}

ParseResult requestData::parse_OneLine()
{
    string::size_type pos = content.find("\r\n", now_read_pos);
    if(pos == string::npos)
        return ParseResult::AGAIN;
    // 得到请求行
    string request_line = content.substr(0 ,pos);
    content.erase(0, pos + 2);// \r\n可以删除了
    // 分析请求行

    // 得到http method
    string md;
    size_t i = 0;
    while(i < request_line.size() && request_line[i] != ' ')
        md += request_line[i++];
    if(md == "GET")
        method = httpMethod::GET;
    else if(md == "POST")
        method = httpMethod::POST;
    else
        return ParseResult::ERROR;

    // 得到请求文件
    if(i == request_line.size())
        return ParseResult::ERROR;
    ++i;
    if(request_line[i] != '/')
        return ParseResult::ERROR;
    while(request_line[i] != ' ' && i < request_line.size())
        file_name.push_back(request_line[i++]);
    // 看下是否有查询字符串, 也就是额外的参数
    pos = file_name.find('?');
    if(pos != string::npos)
        file_name = file_name.substr(0, pos); // 这里选择忽略额外参数
    if(file_name == "/")
        file_name = "/index.html";

    // 得到HTTP版本号
    // 空格HTTP/1.1 总共8个字符
    string ver = request_line.substr(i + 1);
    if(ver.size() != 8)
        return ParseResult::ERROR;
    httpVer = ver.substr(5);
    if(httpVer != "1.0" && httpVer != "1.1")
        return ParseResult::ERROR;
    //  下一个状态, 解析头部字段
    state = ParseRequest::PARSE_HEADERS;
    return ParseResult::SUCCESS;
}

ParseResult requestData::parse_Headers()
{
    return ParseResult::SUCCESS;
}

ParseResult requestData::analysisRequest()
{
    return ParseResult::SUCCESS;
}

// 处理http请求，一切的起点
void requestData::handleRequest()
{
    char buf[4096];
    bool isError = false;
    while(true)
    {
        int readSum = readn(clientFd, buf, 4096);// 读数据
        if(readSum < 0)
        {
            isError = true;
            break;
        }
        else if(readSum == 0)
        {
            // 有请求但是读不到数据，可能是Request Aborted, 或者是对方的数据还没有到达
            if(errno == EAGAIN || errno == EWOULDBLOCK)
            {// 到达一定次数, 就不再尝试
                if(againTime > AGAIN_MAX_TIMES)
                    isError = true;
                else
                    againTime++;
            }
            else if(errno != 0)
                isError = true;
            else
                break;// 对端关闭, 也会返回0, 但这不是错误
            break;
        }
        // 将读到的数据添加到content成员后面
        content += std::string(buf, buf + readSum);

        // 状态机解析
        if(this->state == ParseRequest::PARSE_ONELINE)
        {// 处于解析请求头的状态
            ParseResult flag = parse_OneLine();
            if(flag == ParseResult::AGAIN)
                continue; // 重新进行while循环, 再尝试一次readn
            else if(flag == ParseResult::ERROR)
            {
                isError = true;
                break;
            }
            else
                ;
        }
        // 解析头部字段
        if(this->state == ParseRequest::PARSE_HEADERS)
        {
            ParseResult flag = parse_Headers();
            if(flag == ParseResult::AGAIN)
                continue;
            else if(flag == ParseResult::ERROR)
            {
                isError = true;
                break;
            }
            else
                ;
            if(method == httpMethod::POST)
                state = ParseRequest::PARSE_BODY;
            else
                state = ParseRequest::ANALYSIS;
        }
    }
}

void requestData::handleError(int fd, int err_num, std::string short_msg)
{// 我觉得这函数写的贼垃圾，我可以写的更好
    short_msg = " " + short_msg;
    char send_buf[4096];
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
    //writen(fd, send_buf, strlen(send_buf));// 写入header
    sprintf(send_buf, "%s", body_buff.c_str());
    //writen(fd, send_buf,strlen(send_buf));// 写入body
}