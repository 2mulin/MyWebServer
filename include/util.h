/***********************************************************
 *@author RedDragon
 *@date 2020/9/1
 *@brief 工具，一些函数
***********************************************************/

#ifndef WEBSERVER_UTIL_H
#define WEBSERVER_UTIL_H

#include <cstdlib>
#include <csignal>
#include <fcntl.h>
#include <unistd.h>
#include <cerrno>

// 改变对SIGPIPE信号的处理为忽视
void handlerForSIGPIPE();

// 设置fd为非阻塞
int setSocketNonBlocking(int fd);

// readn,保证读到n个字节
ssize_t readn(int fd, void* buff, std::size_t n);

// writen，保证写n个字节
ssize_t writen(int fd, const void* buff, std::size_t n);

#endif //WEBSERVER_UTIL_H
