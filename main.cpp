#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string>
#include <deque>
#include <queue>

#include "useEpoll.h"
#include "requestData.h"
#include "util.h"
#include "threadpool.h"
using namespace std;

extern pthread_mutex_t qlock;       // request.cpp中定义的互斥量
extern priority_queue<mytimer *, vector<mytimer *>, timerCmp> myTimeQueue;// requestData.cpp定义的定时器队列

const int THREADPOOL_THREAD_COUNT = 4;
const int QUEUE_SIZE = 65535;

const uint16_t PORT = 23456;
const int ASK_STATIC_FILE = 1;// 请求静态文件
const int ASK_IMAGE_STITCH = 2;

// epoll要用的东西
const int LISTENQ = 1024;
const int MAXEVENTS = 5000;

// index.html文件存放路径
const string PATH = "/home/MyWebServer/";

const int TIMER_TIME_OUT = 6000000;// 计时器超时时间（毫秒）

// 根据指定端口创建soket，并且bind，listen
int socket_bind_listen(uint16_t port)
{
    // 检查port值，取正确区间范围
    if(port < 1024 || port > 65535)
        return -1;

    // 创建socket(IPv4 + TCP), 返回监听描述符
    int listen_fd = 0;
    if((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        return -1;

    // 消除bind时"Adress a'lready is used"的错误，
    int optval = 1;
    // SOL_SOCKET开启socket选项，SO_REUSEADDR选项功能是打开或关闭地址复用功能，这里的optval为1，表示打开
    if(setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1)
        return -1;

    // 设置服务器的IP和port，并且绑定到监听的socket描述符上。
    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    // 主机字节序转化成网络字节序
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);// IP地址
    server_addr.sin_port = htons(port);// 端口号

    cout << "server port: " << port << endl;

    if(bind(listen_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1)
        return -1;


    // 开始监听，第二参数：未决的连接个数（TLPI 951页）
    if(listen(listen_fd, LISTENQ) == -1)
        return -1;

    if(listen_fd == -1)
    {// 无效监听描述符
        close(listen_fd);
        return -1;
    }
    return listen_fd;
}

// 多线程必须是这个格式的函数指针。
void myHandler(void *arg)
{
    requestData *req_data = (requestData*)arg;
    req_data->handleRequest();
}

// 处理客户端的连接请求处理，全部放到epoll实例中去
int acceptConnection(int listen_fd, int epoll_fd, const string &path)
{
    int acceptSum = 0;// 客户端总共连接数
    struct sockaddr_in client_addr;
    memset(&client_addr, 0, sizeof(struct sockaddr_in));
    socklen_t client_addr_len = sizeof(client_addr);
    int accept_fd;// 接受到的客户端socket文件描述符

    // 处理所有连接请求（listen_fd是非阻塞的）
    while((accept_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &client_addr_len)) > 0)
    {
        /******************************************************
         * tcp的保活机制默认是关闭的
         * int optval = 0;
         * socklen_t len_optval = 4;
         * getsockopt(accept_fd, SOL_SOCKET,  SO_KEEPALIVE, &optval, &len_optval);
         * cout << "optval ==" << optval << endl;
        ********************************************************/

        acceptSum++;
        // 将通过accept得到的客户端fd设置为非阻塞模式
        if(setSocketNonBlocking(accept_fd) < 0)
        {
            perror("accept_fd setNonBlock Failed!");
        }
        // 增加客户端的socket文件描述符到epoll实例中去
        requestData *req_info = new requestData(accept_fd, epoll_fd, path);
        epoll_add(epoll_fd, accept_fd, req_info, EPOLLIN | EPOLLET | EPOLLONESHOT);
        // 新增计时器，到期时间设置为 nowTime + TIMER_TIME_OUT
        mytimer *mtimer = new mytimer(req_info, TIMER_TIME_OUT);
        req_info->addTimer(mtimer);

        pthread_mutex_lock(&qlock);
        myTimeQueue.push(mtimer);
        pthread_mutex_unlock(&qlock);
    }
    return acceptSum;
}

// 处理epoll实例中的活跃事件, epoll_wait()会将所有触发的事件放到evArray数组中
void handle_events(int epoll_fd, int listen_fd, struct epoll_event *evArray, int events_num, const string &path, threadpool_t* tp)
{
    for(int i = 0; i < events_num; i++)
    {
        // EPOLLERR==有错误发生 EPOLLHUB==出现挂断
        if((evArray[i].events & EPOLLERR)
           || (evArray[i].events & EPOLLHUP) )
        {
            cout << "fd=" << evArray[i].data.fd << " error event\n" << endl;
            continue;
        }

        if(evArray[i].data.fd == listen_fd)
        {// 服务器fd 活跃
            int count = acceptConnection(listen_fd, epoll_fd, path);
            cout << "服务器IO事件触发：收到" << count << "个客户端连接请求" << endl;
        }
        else
        {// 客户端fd 活跃

            cout << "客户端IO事件触发" << endl;
            // 获取有事件产生的描述符, data.ptr指向一个包含fd的struct或class对象
            requestData* request = (requestData*)evArray[i].data.ptr;
            // 加入线程池之前将Timer和request分离
            request->separateTimer();

            // 将请求任务加入到线程池中
            int ret = threadpool_add(tp, myHandler, evArray[i].data.ptr);
            if(ret < 0)
                cout << "线程池添加任务失败" << endl;
        }
    }
}

/* 处理逻辑是这样的~
因为
(1) 优先队列不支持随机访问
(2) 即使支持，随机删除某节点后破坏了堆的结构，需要重新更新堆结构。
所以对于被置为deleted的时间节点，会延迟到它(1)超时 或 (2)它前面的节点都被删除时，它才会被删除。
一个点被置为deleted,它最迟会在TIMER_TIME_OUT时间后被删除。
这样做有两个好处：
(1) 第一个好处是不需要遍历优先队列，省时。
(2) 第二个好处是给超时时间一个容忍的时间，就是设定的超时时间是删除的下限(并不是一到超时时间就立即删除)，如果监听的请求在超时后的下一次请求中又一次出现了，
就不用再重新申请requestData节点了，这样可以继续重复利用前面的requestData，减少了一次delete和一次new的时间。
*/

// 处理过期事件
void handle_expired_event()
{
    pthread_mutex_lock(&qlock);
    while(!myTimeQueue.empty())
    {
        // delete timer就会调用timer的析构函数，析构函数执行了delete timer绑定的requestData的操作
        mytimer *ptimer_now = myTimeQueue.top();
        if(ptimer_now->isDeleted())
        {
            myTimeQueue.pop();
            delete ptimer_now;
        }
        else if(!ptimer_now->isValid())
        {// 超时（不合法）
            myTimeQueue.pop();
            delete ptimer_now;
        }
        else
        {
            break;
        }
    }
    pthread_mutex_unlock(&qlock);
}

int main()
{
    // 忽视SIGPIPE信号
    struct sigaction sa;
    sa.sa_flags = 0;
    sa.sa_handler = SIG_IGN;// 忽视
    if (sigaction(SIGPIPE, &sa, nullptr))
    {
        perror("sigaction() failed");
        return -1;
    }

    // 以线程数，队列大小，位掩码创建一个线程池，等待执行任务
    threadpool_t* threadpool = threadpool_create(THREADPOOL_THREAD_COUNT, QUEUE_SIZE);
    if(threadpool == nullptr)
    {
        perror("thread_create() failed");
        return -1;
    }

    // 创建socket，bind并且开启监听
    int listen_fd = socket_bind_listen(PORT);
    if(listen_fd < 0)
    {
        perror("socket_bind_listen failed!");
        return -1;
    }
    // 将监听socket设置为非阻塞
    if(setSocketNonBlocking(listen_fd) < 0)
    {
        perror("set Socket Non Blocking failed");
        return -1;
    }
    // 创建一个epoll实例
    int epoll_fd = epoll_create(LISTENQ + 1);// size参数并不是一个上限，只是告诉内核可能是size个连接，准备好大小足够的数据结构
    if(epoll_fd < 0)
    {
        perror("epoll_init() failed");
        return -1;
    }

    // 将服务器的socket_fd添加到epoll实例中
    struct epoll_event server_event;
    server_event.events = EPOLLIN | EPOLLET;
    server_event.data.fd = listen_fd;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_fd, &server_event);

    // 初始化evArray数组
    struct epoll_event* evArray = new struct epoll_event[MAXEVENTS];

    int count = 3;
    int events_num = 0;// 触发事件数
    while(events_num >= 0)
    {
        // 得到listen_fd上IO就绪事件的个数（events_num）和 evArray（IO就绪事件数组）
        events_num = my_epoll_wait(epoll_fd, evArray, MAXEVENTS, -1);
        if(events_num <= 0)
            continue;
        cout << "就绪事件数：" <<  events_num << endl;
        //cout << "计时器队列大小：" << myTimeQueue.size() << endl;

        if(count > 0)
        {
            // 遍历evArray数组, 找到活跃事件，根据监听种类及描述符类型分别操作
            handle_events(epoll_fd, listen_fd, evArray, events_num, PATH, threadpool);
            count--;
        }

        cout << endl;
        // 处理过期事件
        // handle_expired_event();
    }

    delete[] evArray;
    evArray = nullptr;
    return 0;
}
