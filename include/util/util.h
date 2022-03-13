/***********************************************************
 *@author ling
 *@date 2021/5/23
 *@brief 工具，一些函数,类
***********************************************************/

#ifndef WEBSERVER_UTIL_H
#define WEBSERVER_UTIL_H

#include <string>
#include <cstdint>
#include <cxxabi.h>

/**
 * @brief 禁止拷贝
 */
class nonCopyAble
{
public:
    nonCopyAble() = default;
    ~nonCopyAble() = default;

private:
    /// 禁用拷贝，也可以私有化拷贝构造函数和拷贝赋值运算符
    nonCopyAble(const nonCopyAble&) = delete;
    nonCopyAble& operator=(const nonCopyAble&) = delete;
};

const int LISTENQ = 1024;

/**
 * @brief 设置信号处理方式为忽视
 * @param sig 信号
 */
void setSigIgn(int sig);

/**
 * @brief 根据指定端口开启监听
 * @param port 指定端口
 * @return 成功返回listen_fd，失败返回-1
 */
int socket_bind_listen(int port);

/**
 * @brief 设置指定fd为非阻塞模式
 * @param fd 指定fd
 * @return 成功返回0，失败返回-1
 */
int setNonBlock(int fd);

/**
 * @brief 多次调用read，读size个字节，直到对端关闭或者出现错误
 */
int readn(int fd, char* buf, size_t size);

/**
 * @brief 多次调用write，写size个字节，直到对端关闭或者出现错误。
 */
int writen(int fd, const char *buf, size_t size);

/**
 * @brief 根据文件后最得到文件类型名
 * @param suffix 文件后缀形式
 * @return 文件类型
 */
std::string getMimeType(const std::string& suffix);

/**
 * @brief 获得当前时间(毫秒级)
 */
std::uint64_t current_time();

/**
 * @brief 将类型名转化为字符串(typeinfo)
 * @tparam T
 * @return char* 类型名字符串
 */
template<typename T>
const char* typeToName()
{
    static const char* name = abi::__cxa_demangle(typeid(T).name(), nullptr, nullptr, nullptr);
    return name;
}

#endif //WEBSERVER_UTIL_H
