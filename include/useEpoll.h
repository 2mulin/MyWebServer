/***********************************************************
 *@author RedDragon
 *@date 2020/8/26
 *@brief 封装了一些使用epoll的函数
***********************************************************/
#ifndef WEBSERVER_USEEPOLL_H
#define WEBSERVER_USEEPOLL_H

#include <sys/epoll.h>
#include <cerrno>
#include <cstdio>// perror头文件

int epoll_add(int epollfd,int fd,void* request,uint32_t events);
int epoll_mod(int epoll_fd,int fd,void* request,uint32_t events);
int epoll_del(int epoll_fd,int fd,void* request,uint32_t events);
int my_epoll_wait(int epoll_fd, struct epoll_event* events, int max_events, int timeout);


#endif //WEBSERVER_USEEPOLL_H
