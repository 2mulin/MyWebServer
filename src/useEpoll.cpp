/***********************************************************
 *@author RedDragon
 *@date 2020/8/26
 *@brief 定义了一些使用epoll的函数
 * 这里是直接写的函数，后面可以用class封装一下
***********************************************************/
#include "useEpoll.h"

// 将fd添加到epoll_fd实例的interest列表中
int epoll_add(int epoll_fd, int fd, void *request, uint32_t events)
{
    struct epoll_event ev;
    // 两个都指定
    ev.data.fd = fd;
    ev.data.ptr = request;
    ev.events = events;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev) == -1) {
        perror("epoll_add failed");
        return -1;
    }
    return 0;
}

// 修改fd状态
int epoll_mod(int epoll_fd, int fd, void *request, uint32_t events)
{
    struct epoll_event event;
    event.events = events;
    event.data.ptr = request;

    if(epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &event) == -1){
        perror("epoll_mod failed");
        return -1;
    }
    return 0;
}

// 从epoll实例中移除fd
int epoll_del(int epoll_fd, int fd, void *request, uint32_t events)
{
    struct epoll_event event;
    event.events = events;
    event.data.ptr = request;

    if(epoll_ctl(epoll_fd,EPOLL_CTL_DEL,fd,&event) == -1){
        perror("epoll_del failed");
        return -1;
    }
    return 0;
}

// 返回活跃事件数，evList指向的结构体数组中会返回有关就绪态文件描述符的信息。
int my_epoll_wait(int epoll_fd, struct epoll_event* evArray, int max_events, int timeout)
{
    int ret_count = epoll_wait(epoll_fd,evArray,max_events,timeout);
    if(ret_count < 0)
    {
        // 完成之后记得把这里删除 两句删除

        // gdb调试时会命中断点，导致程序会收到一个SIGTRAP信号，epoll_wait这种阻塞的函数可能会收到这个信号，导致返回非0值, 进程退出
        if(errno == EINTR)
            return 0;
        perror("epoll_wait() error");
    }
    return ret_count;
}