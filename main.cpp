#include <sys/socket.h>
#include <arpa/inet.h>
#include <queue>

#include "useEpoll.h"
#include "requestData.h"
#include "util.h"
#include "threadpool.h"
#include "Lock.h"

const int PORT = 23456;

const int ASK_STATIC_FILE = 1;// 请求静态文件
const int ASK_IMAGE_STITCH = 2;

const int MAXEVENTS = 5000;// epoll_event数组大小

// index.html文件存放路径
const string PATH = "/home/MyWebServer/";

const int TIMER_TIME_OUT = 500;// 计时器超时时间（毫秒）

// 添加到任务队列的函数
void handler(void *arg)
{
    if(arg == nullptr){
        printf("arg参数错误!\n");
        return;
    }
    requestData *req = (requestData*)arg;
    // 执行真正的处理函数
    req->handleRequest();
}

// 处理客户端的连接请求
int acceptConn(std::priority_queue<requestData*>& qu, int listen_fd, int epoll_fd)
{
    struct sockaddr_in clientAddr;
    socklen_t socklen = sizeof(clientAddr);
    int fd = 0;
    // 由于是边缘触发, listen_fd读就绪时, 可能有多个请求到达.
    while((fd = accept(listen_fd, (struct sockaddr* )&clientAddr, &socklen)) > 0){
        setNonBlock(fd);
        // 构造函数会将fd添加到epoll_fd中.
        requestData* p = new requestData(fd, epoll_fd, PATH);
        Lock();
        qu.push(p);
    }
    return 0;
}

// 处理队列中过期的requestData和已经完成的requestData
void handleTimeout(std::priority_queue<requestData*> qu)
{
    Lock();
    while(!qu.empty())
    {
        requestData* ptr = qu.top();
        if(ptr->isTimeout())
        {
            qu.pop();
            delete ptr;
        }
        else
            break;
    }
}

int main()
{
    // 线程池初始化(静态变量)
    try
    {}
    catch (const std::runtime_error& error)
    {
        printf("%s\n", error.what());
        return -1;
    }

    // 忽视SIGPIPE信号(如果客户端突然关闭, 那么服务器write就会碰到一个SIGPIPE信号)
    handlerForSIGPIPE();
    int listen_fd = socket_bind_listen(PORT);
    if(listen_fd == -1)
    {
        perror("socket_bind_listen");
        return -1;
    }
    // 将监听socket设置为非阻塞搭配边缘触发
    if(setNonBlock(listen_fd) == -1)
    {
        perror("setNonBlock");
        return -1;
    }
    threadPool &pool = threadPool::getInstance();
    printf("创建线程池成功!\n");
    // epoll初始化
    int epoll_fd = epoll_init();
    struct epoll_event ev;
    ev.data.fd = listen_fd;
    ev.events = EPOLLIN | EPOLLET;
    epoll_add(epoll_fd, listen_fd, &ev);

    struct epoll_event epevList[MAXEVENTS];     // 存放epoll_wait返回的事件
    std::priority_queue<requestData*> req_queue;// 请求队列
    while(true)
    {
        int events_num = epoll_wait(epoll_fd, epevList, MAXEVENTS, -1);
        if(events_num == -1)
        {
            perror("epoll_wait");
            break;
        }
        // 检查返回的所有活跃事件
        for(int i = 0; i < events_num; ++i)
        {
            if(epevList[i].data.fd == listen_fd)
            {
                if(-1 == acceptConn(req_queue, listen_fd, epoll_fd))
                    perror("acceptConn");
            }
            else
            {
                requestData *ptr = (requestData*)epevList[i].data.ptr;
                if(epevList[i].events & EPOLLIN)
                {
                    int ret = pool.addTask(task{handler, ptr});
                    if(ret < 0)
                        printf("任务添加失败!\n");
                }
                else if(epevList[i].events & EPOLLERR)
                    printf("EPOLLERR!\n");
                else if(epevList[i].events & EPOLLHUP)
                    printf("EPOLLHUP!\n");
                else if(epevList[i].events & EPOLLRDHUP)
                    printf("EPOLLRDHUP!\n");
            }
        }
        // 处理过期事件
        handleTimeout(req_queue);
    }
    return 0;
}
