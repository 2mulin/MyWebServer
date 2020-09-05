#include "useEpoll.h"
#include "requestData.h"
#include "util.h"
#include "threadpool.h"
using namespace std;


int main(int argc, char *argv[])
{
    handlerForSIGPIPE();
    int epoll_fd = epoll_init();
    if(epoll_fd < 0)
    {
        perror("epoll init failed");
        return 1;
    }


    return 0;
}
