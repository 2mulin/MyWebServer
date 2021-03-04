/***********************************************************
 *@author RedDragon
 *@date 2020/9/1
 *@brief 工具，一些函数
***********************************************************/

#ifndef WEBSERVER_UTIL_H
#define WEBSERVER_UTIL_H

#include <cstdio>
#include <cstring>
#include <string>
#include <unordered_map>

#include <csignal>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

const int LISTENQ = 1024;

// 设置忽略SIGPIPE信号
void handlerForSIGPIPE();
// 开启服务器监听
int socket_bind_listen(int port);
int setNonBlock(int fd);
int readn(int fd, char* buf, size_t size);

// 根据后缀得到文件类型
std::string getMimeType(const std::string& suffix);
#endif //WEBSERVER_UTIL_H
