#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <unistd.h>
#include <cstring>
#include <list>
using std::list;

//#include "timerList.h"1
#include "threadpool.h"
#include "util.h"
#include "useEpoll.h"
#include "httpData.h"
#include "Lock.h"
#include "Timer.h"

const int MAXEVENTS = 5000; // epoll_event数组大小
int epollFd = -1;           // EPOLL实例描述符
const long long TIMEOUT = 30000; // 超时时间
list<Timer*> timerList;  // 所有客户端连接

void task(void *arg);
int acceptConnection(int listen_fd, const string& resPath);

int main(int argc, char* argv[])
{
    // 线程池初始化(静态变量)
    try
    {}
    catch (const std::runtime_error& error)
    {
        printf("%s\n", error.what());
        return -1;
    }
    int PORT = 23456;           // 默认端口号
    char buf[128] = {0};        // 默认资源文件目录
    getcwd(buf, sizeof(buf));
    string resPath;
    for(size_t i = 0; i < sizeof(buf) && buf[i] != '\0'; ++i)
        resPath.push_back(buf[i]);
    int ch = -1;
    while((ch = getopt(argc, argv, "P:R:")) != -1)
    {
        switch(ch)
        {
            case 'P':
                PORT = atoi(optarg);
                if(PORT < 0 || PORT > 65535)
                {
                    printf("端口号设置错误!, 0 > PORT < 65535\n");
                    return -1;
                }
                break;
            case 'R':
                if(access(optarg, F_OK) == -1)
                {
                    printf("资源目录不存在!\n");
                    return -1;
                }
                resPath = string(realpath(optarg, nullptr));
                break;
            case '?':
                printf("usage: %s -P num -R ResourcesPath\n", basename(argv[0]));
                break;
            default:
                break;
        }
    }
    printf("目录: %s\n", resPath.data());

    // 忽视SIGPIPE信号(如果客户端突然关闭读端, 那么服务器write就会碰到一个SIGPIPE信号)
    handlerForSIGPIPE();
    int listen_fd = socket_bind_listen(PORT);
    if(listen_fd == -1)
    {
        perror("socket_bind_listen");
        return -1;
    }
    // 将监听socket设置为非阻塞搭配边缘触发, 这样while(1)accept();就不会阻塞了
    if(setNonBlock(listen_fd) == -1)
    {
        perror("setNonBlock");
        return -1;
    }
    // 线程池初始化
    threadPool& pool = threadPool::getInstance();
    printf("创建线程池成功!\n");
    // 获取epoll实例
    epollFd = epoll_init();
    // 添加listen_fd到epoll实例中去
    struct epoll_event ev;
    ev.data.fd = listen_fd;
    ev.events = EPOLLIN | EPOLLET;
    epoll_add(listen_fd, &ev);

    struct epoll_event epevList[MAXEVENTS];     // 存放epoll_wait返回的事件
    while(true)
    {
        int events_num = epoll_wait(epollFd, epevList, MAXEVENTS, -1);
        // EINTR中断不算错误, 比如被信号中断
        if(events_num == -1 && errno != EINTR)
        {
            perror("epoll_wait");
            break;
        }
        // 检查返回的所有活跃事件
        for(int i = 0; i < events_num; ++i)
        {
            if(epevList[i].data.fd == listen_fd)
            {// 监听socket触发
                if(-1 == acceptConnection(listen_fd, resPath))
                    perror("acceptConn");
            }
            else
            {// 客户端socket触发
                httpData* data = (httpData*) epevList[i].data.ptr;
                if(epevList[i].events & EPOLLIN)
                {
                    // 添加任务之前把指针断开
                    Timer* p = data->timer;
                    data->timer = nullptr;
                    p->data = nullptr;
                    if(pool.addTask(threadPool_task{task, data}) < 0)
                    {
                        printf("任务添加失败, 任务队列满了!\n");
                        return -1;
                    }
                    else
                    {
                    }
                }
                else if(epevList[i].events & EPOLLERR)
                    printf("EPOLLERR!\n");
                else if(epevList[i].events & EPOLLHUP)
                    printf("EPOLLHUP!\n");
            }
        }
        // 处理超时连接
        // timerlist.tick();
        struct timeval now;
        gettimeofday(&now, nullptr);
        long long val = (long long)now.tv_sec * 1000 + (long long)now.tv_usec / 1000;
        Lock();
        while(!timerList.empty())
        {
            Timer* tim = timerList.front();
            if(tim->expiredTime >= val)
                timerList.pop_front();
            if(tim->data == nullptr)
                timerList.pop_front();
        }
    }
    return 0;
}

// 添加到任务队列的函数
void task(void *arg)
{
    if(arg == nullptr)
        exit(-1);
    httpData* data = (httpData*)arg;
    ParseRequest ret = data->handleRequest();
    if(ret == ParseRequest::FINISH || ret == ParseRequest::ERROR)
        delete data;
}

// 处理连接请求
int acceptConnection(int listen_fd, const string& resPath)
{
    struct sockaddr_in clientAddr;
    socklen_t socklen = sizeof(clientAddr);
    int fd = -1;
    // 由于是边缘触发, listen_fd读就绪时, 可能有多个请求到达.
    while((fd = accept(listen_fd, (struct sockaddr* )&clientAddr, &socklen)) > -1){
        setNonBlock(fd);
        httpData* httpdata = new httpData(fd, TIMEOUT, resPath);
        Timer* timer = new Timer(TIMEOUT);
        // 指针互指
        httpdata->timer = timer;
        timer->data = httpdata;

        struct epoll_event ev;
        ev.data.ptr = httpdata;
        ev.events = EPOLLET | EPOLLIN | EPOLLONESHOT;
        epoll_add(fd, &ev);

        // 添加计时器
        Lock();
        timerList.push_back(timer);
    }
    return 0;
}