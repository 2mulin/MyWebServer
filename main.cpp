#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <csignal>
#include <cstring>

#include "threadpool.h"
#include "util.h"
#include "useEpoll.h"
#include "httpData.h"
#include "Lock.h"
#include "Timer.h"

const int MAXEVENTS = 4096; // epoll_event数组大小
int epollFd = -1;           // EPOLL实例描述符
const uint64_t TIMEOUT = 30000;  // 连接超时时间
TimerManager timerQueue;    // 所有计时器

void task(void *arg);

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
                return -1;
            default:
                break;
        }
    }
    printf("资源目录: %s\n", resPath.data());

    // 忽视SIGPIPE信号(如果客户端突然关闭读端, 那么服务器write就会碰到一个SIGPIPE信号)
    setSigIgn(SIGPIPE);
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
    int64_t epoll_timeout = -1;                     // 利用epoll_wait实现定时器
    while(true)
    {
        int events_num = epoll_wait(epollFd, epevList, MAXEVENTS, epoll_timeout);
        // EINTR中断不算错误, 比如被信号中断
        if(events_num == -1 && errno != EINTR)
        {
            perror("epoll_wait");
            break;
        }
        else if(events_num == 0)
            timerQueue.takeAllTimeout();
        else
        {
            for(int i = 0; i < events_num; ++i)
            {
                if(epevList[i].data.fd == listen_fd)
                {// 监听socket触发
                    struct sockaddr_in clientAddr;
                    socklen_t socklen = sizeof(clientAddr);
                    int fd = -1;
                    // 由于是边缘触发, listen_fd读就绪时, 可能有多个请求到达.
                    while((fd = accept(listen_fd, (struct sockaddr* )&clientAddr, &socklen)) > -1){
                        setNonBlock(fd);
                        httpData* data = new httpData(fd, resPath);
                        struct epoll_event ev;
                        ev.data.ptr = data;
                        ev.events = EPOLLET | EPOLLIN | EPOLLONESHOT;
                        epoll_add(fd, &ev);
                        // 绑定一个计时器, 回调函数的任务就是 delete httpdata
                        data->timer = timerQueue.addTimer(TIMEOUT, [data,&ev](){
                            epoll_del(data->getFd(), &ev);
                            close(data->getFd());
                            delete data;
                        });
                    }
                }
                else
                {// 客户端socket触发
                    httpData* data = (httpData*) epevList[i].data.ptr;
                    if(epevList[i].events & EPOLLIN)
                    {// 添加任务时断开计时器
                        data->timer->cancel();// 取消timer的任务
                        data->timer = nullptr;
                        if(pool.addTask(threadPool_task{task, data}) < 0)
                        {
                            printf("任务添加失败, 任务队列满了!\n");
                            break;
                        }
                    }
                    else if(epevList[i].events & EPOLLERR)
                        printf("EPOLLERR!\n");
                    else if(epevList[i].events & EPOLLHUP)
                        printf("EPOLLHUP!\n");
                }
            }
        }
        epoll_timeout = timerQueue.getMinTO();
        if(epoll_timeout < -1)
        {// 已经有定时器到期了
            timerQueue.takeAllTimeout();
            epoll_timeout = timerQueue.getMinTO();
        }
    }
    return 0;
}

// 添加到任务队列的函数
void task(void *arg)
{
    if(arg == nullptr)
        exit(-1);// 线程中使用, 整个进程直接退出
    httpData* data = (httpData*)arg;
    ParseRequest ret = data->handleRequest();

    struct epoll_event ev;
    ev.data.ptr = data;
    ev.events = EPOLLIN | EPOLLET | EPOLLONESHOT;

    if(ret == ParseRequest::KEEPALIVE)
    {// 是长连接, 重置
        data->reset();
        // 重新添加计时器和回调函数
        data->timer = timerQueue.addTimer(TIMEOUT, [data, &ev](){
            epoll_del(data->getFd(), &ev);
            close(data->getFd());
            delete data;
        });
        // 先添加定时器在激活, 否则可能激活EPOLLONESHOR, 马上就触发了, timer还没加上去
        epoll_mod(data->getFd(), &ev);        // 重新激活EPOLLONESHOT
    }
    if(ret == ParseRequest::FINISH || ret == ParseRequest::ERROR)
    {//不能放到timer回调中,必须马上epoll_del, 否则回调还没做, 可能又触发了
        epoll_del(data->getFd(), &ev);
        close(data->getFd());
        delete data;
    }
}