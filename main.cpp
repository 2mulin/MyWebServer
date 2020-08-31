#include <iostream>
#include <csignal>
#include <fcntl.h>
#include "useEpoll.h"
#include "requestData.h"
using namespace std;

// 改变对SIGPIPE信号的处理为忽视
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
    // 得到已打开文件fd的状态标志
    int flags = fcntl(fd, F_GETFL);
    if (flags == -1)
        return -1;

    flags |= O_NONBLOCK;// 加上非阻塞
    // 设置成非阻塞
    if (fcntl(fd,F_SETFD,flags) == -1)
        return -1;
    return 0;
}

// readn

// writen

int main(int argc, char *argv[])
{
    handlerForSIGPIPE();
    int epoll_fd = epoll_init();
    
    return 0;
}
