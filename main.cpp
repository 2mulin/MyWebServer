#include "useEpoll.h"
#include "requestData.h"
#include "util.h"
using namespace std;


int main(int argc, char *argv[])
{
    handlerForSIGPIPE();
    int epoll_fd = epoll_init();
    
    return 0;
}
