/***********************************************************
 *@author RedDragon
 *@date 2020/8/26
 *@brief 定义了一些使用epoll的函数
 * 这里是直接写的函数，后面可以用class封装一下
***********************************************************/
#include "useEpoll.h"

// 初始化一个epoll实例
int epoll_init()
{
    int epoll_fd = epoll_create(10);// 参数已经没有效果了
    if(epoll_fd == -1)
        perror("epoll_create");
    return epoll_fd;
}

int epoll_add(int fd, struct epoll_event* ev)
{
    int ret = epoll_ctl(epollFd, EPOLL_CTL_ADD, fd, ev);
    if (ret == -1) {
        perror("epoll_add");
        return -1;
    }
    return 0;
}

// 修改fd状态
int epoll_mod(int fd, struct epoll_event* ev)
{
    if(epoll_ctl(epollFd, EPOLL_CTL_MOD, fd, ev) == -1){
        perror("epoll_mod failed");
        return -1;
    }
    return 0;
}

// 从epoll实例中移除fd
int epoll_del(int fd, struct epoll_event* ev)
{
    if(epoll_ctl(epollFd, EPOLL_CTL_DEL, fd, ev) == -1){
        perror("epoll_del failed");
        return -1;
    }
    return 0;
}
