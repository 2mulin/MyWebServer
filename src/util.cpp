/***********************************************************
 *@author RedDragon
 *@date 2020/9/1
 *@brief 
***********************************************************/
#include "util.h"

void handlerForSIGPIPE()
{
    struct sigaction sa{};
    sa.sa_flags = 0;
    sa.sa_handler = SIG_IGN;// 忽视
    if (sigaction(SIGPIPE, &sa, nullptr))
        return;
}

// 设置fd为非阻塞
int setSocketNonBlocking(int fd)
{
    // 得到已打开文件fd的状态标志位
    int flags = fcntl(fd, F_GETFL);
    if (flags == -1)
        return -1;

    flags |= O_NONBLOCK;// 加上非阻塞
    // 设置成非阻塞
    if (fcntl(fd, F_SETFD, flags) == -1)
        return -1;
    return 0;
}

// readn,保证读到n个字节。因为socket缓冲区有大小限制，可能不能一次性读出n个字节
ssize_t readn(int fd, void *buff, std::size_t n)
{
    size_t nleft = n;// 还剩nleft字节没读
    char *ptr = (char *) buff;
    while (nleft > 0)
    {
        ssize_t nread = read(fd, ptr, nleft);
        if (nread < 0)
        {
            if (errno == EINTR)
                nread = 0;// 被中断了，重新再读一次
            else
                return -1;// 其他错误，退出
        }
        else if (nread == 0)
            break;// 缓冲区读完了，退出
        nleft -= nread;
        ptr += nread;// 指针后移
    }
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