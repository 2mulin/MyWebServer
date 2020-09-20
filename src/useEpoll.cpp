/***********************************************************
 *@author RedDragon
 *@date 2020/8/26
 *@brief 定义了一些使用epoll的函数
 * 这里是直接写的函数，后面可以用class封装一下
***********************************************************/
#include "useEpoll.h"

// epoll_event类型的一个数组，在创建一个epoll实例时初始化
struct epoll_event *events = nullptr;

// 创建一个epoll实例，返回epoll实例
int epoll_init() {
    int epoll_fd = epoll_create(LISTENQ + 1);// size参数并不是一个上限，只是告诉内核可能是size个连接，准备好大小足够的数据结构
    if (epoll_fd == -1)
        return -1;
    events = new struct epoll_event[MAXEVENTS];// events指针初始化
    return epoll_fd;
}

// 将fd添加到epoll_fd实例的interest列表中
int epoll_add(int epoll_fd, int fd, void *request, uint32_t events) {
    struct epoll_event ev;
    ev.events = events;
    ev.data.ptr = request;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev) == -1) {
        perror("epoll_add failed");
        return -1;
    }
    return 0;
}

// 修改fd状态
int epoll_mod(int epoll_fd, int fd, void *request, uint32_t events) {
    struct epoll_event event;
    event.events = events;
    event.data.ptr = request;

    if(epoll_ctl(epoll_fd,EPOLL_CTL_MOD,fd,&event) == -1){
        perror("epoll_mod failed");
        return -1;
    }
    return 0;
}

// 从epoll实例中移除fd
int epoll_del(int epoll_fd, int fd, void *request, uint32_t events) {
    struct epoll_event event;
    event.events = events;
    event.data.ptr = request;

    if(epoll_ctl(epoll_fd,EPOLL_CTL_DEL,fd,&event) == -1){
        perror("epoll_del failed");
        return -1;
    }
    return 0;
}

// 返回活跃事件数，返回epoll_event结构体变量放到events数组中。
int my_epoll_wait(int epoll_fd, struct epoll_event* events, int max_events, int timeout)
{
    int ret_count = epoll_wait(epoll_fd,events,max_events,timeout);
    if(ret_count < 0)
    {
        perror("epoll wait error");
    }
    return ret_count;
}