/***********************************************************
 *@author RedDragon
 *@date 2020/8/26
 *@brief 定义了一些使用epoll的函数
 * 这里是直接写的函数，后面可以用class封装一下
***********************************************************/
#include "useEpoll.h"

struct epoll_event *ev;

// 创建一个epoll实例，返回epoll实例的fd
int epoll_init() {
    int epoll_fd = epoll_create(LISTENQ + 1);// 传size参数好像没什么用了
    if (epoll_fd == -1)
        return -1;
    ev = new struct epoll_event[MAXEVENTS];// ev指针初始化
    return epoll_fd;
}

// 将fd添加到epoll_fd
int epoll_add(int epoll_fd, int fd, void *request, uint32_t events) {
    struct epoll_event event{};
    event.events = events;
    event.data.ptr = request;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event) == -1) {
        perror("epoll_add failed");
        return -1;
    }
    return 0;
}

// 修改fd状态
int epoll_mod(int epoll_fd, int fd, void *request, uint32_t events) {
    struct epoll_event event{};
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
    struct epoll_event event{};
    event.events = events;
    event.data.ptr = request;

    if(epoll_ctl(epoll_fd,EPOLL_CTL_DEL,fd,&event) == -1){
        perror("epoll_del failed");
        return -1;
    }
    return 0;
}