/***********************************************************
 *@author RedDragon
 *@date 2020/9/1
 *@brief 一些使用到的函数
***********************************************************/
#include "util.h"

void handlerForSIGPIPE()
{
    struct sigaction act;
    act.sa_flags = 0;
    act.sa_handler = SIG_IGN;
    sigfillset(&act.sa_mask);// 信号处理函数执行时, 阻塞所有信号
    if(-1 == sigaction(SIGPIPE, &act, nullptr))
        perror("sigaction");
}

// 根据指定端口创建soket，并且bind，listen
int socket_bind_listen(int port)
{
    // 检查port值，取正确区间范围
    if(port < 1024 || port > 65535)
        return -1;

    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(listen_fd == -1)
        return -1;
    // 消除bind时"Adress a'lready is used"的错误，
    int optval = 1;
    if(setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1)
        return -1;

    // 设置服务器的IP和port，并且绑定到监听的socket描述符上。
    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;// IP地址
    server_addr.sin_port = htons(port);// 端口号
    if(bind(listen_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1)
        return -1;

    // 开始监听，第二参数：未决的连接个数（TLPI 951页）
    if(listen(listen_fd, LISTENQ) == -1)
        return -1;

    return listen_fd;
}

int setNonBlock(int fd)
{
    int flag = 0;
    int ret = fcntl(fd, F_GETFL, flag);
    if(-1 == ret)
        return -1;
    flag |= O_NONBLOCK;
    ret = fcntl(fd, F_SETFL, flag);
    if(-1 == ret)
        return -1;
    return 0;
}

std::string getMimeType(const std::string& suffix)
{
    static std::unordered_map<std::string, std::string> map;
    // 防止线程安全, 应该在线程跑起来之前初始化一下map
    if(map.empty())
    {
        map.insert(std::pair<std::string, std::string>(".html","text/html"));
        map.insert(std::pair<std::string, std::string>(".htm","text/html"));
        map.insert(std::pair<std::string, std::string>(".avi","video/x-msvideo"));
        map.insert(std::pair<std::string, std::string>(".bmp","timage/bmp"));
        map.insert(std::pair<std::string, std::string>(".c","text/plain"));
        map.insert(std::pair<std::string, std::string>(".doc","application/msword"));
        map.insert(std::pair<std::string, std::string>(".gif","image/gif"));
        map.insert(std::pair<std::string, std::string>(".gz","application/x-gzip"));
        map.insert(std::pair<std::string, std::string>(".ico","application/x-ico"));
        map.insert(std::pair<std::string, std::string>(".png", "image/png"));
        map.insert(std::pair<std::string, std::string>(".txt", "text/plain"));
        map.insert(std::pair<std::string, std::string>(".mp3", "audio/mp3"));
        map.insert(std::pair<std::string, std::string>(".default", "text/html"));
    }
    auto it = map.find(suffix);
    if(it != map.end())
        return it->second;
    return map[".default"];
}

int readn(int fd, char* buf, size_t size)
{
    int total = 0;
    int index = 0;
    int num = 0;
    while((num = read(fd, buf + index, size)) > 0)
    {
        if(num > 0)
        {
            total += num;
            index += num;
        }
        else if(num == 0)
        {
            // 对端关闭才会读到0
            printf("对端关闭! 读到文件尾\n");
            return total;
        }
        else
        {
            if(errno == EAGAIN || errno == EWOULDBLOCK)
                break;// 对端数据读完了(正常情况), 退出
            else if(errno == EINTR)
                continue;// 被信号中断, 回来继续
            perror("read");
            return -1;
        }
    }
    return total;
}