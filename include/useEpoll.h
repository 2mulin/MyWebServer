/***********************************************************
 *@author RedDragon
 *@date 2020/8/26
 *@brief 封装了一些使用epoll的函数
***********************************************************/
#ifndef WEBSERVER_USEEPOLL_H
#define WEBSERVER_USEEPOLL_H
#include "requestData.h"

const int MAXEVENTS = 5000;
const int LISTENQ = 1024;//监听队列

int epoll_init();
int epoll_add(int epoll_fd,int fd,void* request,__uint32_t events);
int epoll_mod(int epoll_fd,int fd,void* request,__uint32_t events);
int epoll_del(int epoll_fd,int fd,void* request,__uint32_t events);


#endif //WEBSERVER_USEEPOLL_H
