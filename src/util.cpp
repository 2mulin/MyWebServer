/***********************************************************
 *@author RedDragon
 *@date 2020/9/1
 *@brief 
***********************************************************/
#include "util.h"
#include <stdio.h>

// 设置fd为非阻塞
int setSocketNonBlocking(int fd)
{
    /************************************************************
     * 由于fcntl中第二个参数F_SETFL错写成了F_SETFD导致设置非阻塞并未成功
     * 特此记录一下，一定不能写错字母，否则编译调试并不报错，但是没效果
     *
     * F_GETFD 读取文件描述符状态标志
     * F_SETFD 设置文件描述符状态标志
     * F_GETFL 读取文件状态标志
     * F_SETFL 设置文件状态标志
     ************************************************************/

    // 得到已打开文件fd的状态标志位
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1)
        return -1;

    flags |= O_NONBLOCK;
    if (fcntl(fd, F_SETFL, flags) == -1)
        return -1;

    return 0;
}

// readn,保证读到n个字节。因为socket缓冲区有大小限制，n > 缓冲区大小，无法一次性读出n个字节
ssize_t readn(int fd, void *buff, std::size_t n)
{
    size_t nleft = n;// 还剩nleft字节没读
    char *ptr = (char *) buff;
    // 只有数据读完了，或者出错了才会退出循环
    while (nleft > 0)
    {
        ssize_t nread = read(fd, ptr, nleft);
        if (nread < 0)
        {
            printf("%d", errno);
            if (errno == EINTR)
                nread = 0;// 被中断了，重新再读一次
            else if (errno == EAGAIN)
                break;// 退出函数后for循环会再次尝试，尝试达到一定次数就是错误
            else
                return -1;// 其他错误，退出
        }
        else if (nread == 0)
            break;// 缓冲区读完了，退出
        nleft -= nread;
        ptr += nread;// 指针后移
    }

    // 返回实际读到的字节
    return n - nleft;
}

// writen，保证写n个字节。
ssize_t writen(int fd, const void *buff, std::size_t n)
{
    size_t nleft = n;
    char *ptr = (char *) buff;
    while (nleft > 0)
    {
        ssize_t nwritten = write(fd, ptr, nleft);
        if (nwritten <= 0)
        {
            if (nwritten < 0 && errno == EINTR)
                nwritten = 0;// 再写一次
            else
                return -1;// 其他错误
        }
        nleft -= nwritten;
        ptr += nwritten;
    }
    return n;
}