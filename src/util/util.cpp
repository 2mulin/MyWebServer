#include "util/util.h"

#include <cstdio>
#include <cstring>
#include <unordered_map>

#include <sys/time.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <csignal>

namespace util
{
    void setSigIgn(int sig)
    {
        struct sigaction act;
        act.sa_flags = 0;
        act.sa_handler = SIG_IGN;
        /// 信号处理函数执行时, 阻塞所有信号
        sigfillset(&act.sa_mask);
        if (-1 == sigaction(sig, &act, nullptr))
            perror("sigaction");
    }

    int socket_bind_listen(int port)
    {
        /// 检查port值，取正确区间范围
        if (port < 1024 || port > 65535)
            return -1;

        int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (listen_fd == -1)
            return -1;
        /// 消除bind时"Adress a'lready is used"的错误，
        int optval = 1;
        if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1)
            return -1;

        /// 设置服务器的IP和port，并且绑定到监听的socket描述符上。
        struct sockaddr_in server_addr;
        bzero(&server_addr, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;// IP地址
        server_addr.sin_port = htons(port);// 端口号
        if (bind(listen_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1)
            return -1;

        /// 开始监听
        if (listen(listen_fd, 8) == -1)
            return -1;
        else
            printf("服务器启动成功! 监听端口: %d\n", port);
        return listen_fd;
    }

    int setNonBlock(int fd)
    {
        int flag = 0;
        int ret = fcntl(fd, F_GETFL, flag);
        if (-1 == ret)
            return -1;
        flag |= O_NONBLOCK;
        ret = fcntl(fd, F_SETFL, flag);
        if (-1 == ret)
            return -1;
        return 0;
    }

    std::string getMimeType(const std::string &suffix)
    {
        static std::unordered_map<std::string, std::string> ma;
        /// 初始化一下ma, 之后就只能读ma。
        if (ma.empty())
        {
            /// 这个不在标准中, 默认当作文本文件, 可以直接展示
            ma.insert(std::pair<std::string, std::string>("default", "text/plain"));
            ma.insert(std::pair<std::string, std::string>(".html", "text/html"));
            ma.insert(std::pair<std::string, std::string>(".htm", "text/html"));
            ma.insert(std::pair<std::string, std::string>(".avi", "video/x-msvideo"));
            ma.insert(std::pair<std::string, std::string>(".bmp", "timage/bmp"));
            ma.insert(std::pair<std::string, std::string>(".c", "text/plain"));
            ma.insert(std::pair<std::string, std::string>(".doc", "application/msword"));
            ma.insert(std::pair<std::string, std::string>(".gif", "image/gif"));
            ma.insert(std::pair<std::string, std::string>(".gz", "application/x-gzip"));
            ma.insert(std::pair<std::string, std::string>(".ico", "application/x-ico"));
            ma.insert(std::pair<std::string, std::string>(".png", "image/png"));
            ma.insert(std::pair<std::string, std::string>(".txt", "text/plain"));
            ma.insert(std::pair<std::string, std::string>(".mp3", "audio/mp3"));
        }
        auto it = ma.find(suffix);
        if (it != ma.end())
            return it->second;
        return ma["default"];
    }

    int readn(int fd, char *buf, size_t size)
    {
        int total = 0;
        int index = 0;
        int num = 0;
        while (size > 0)
        {
            num = read(fd, (void *) (buf + index), size);
            if (num > 0)
            {
                total += num;
                index += num;
                size -= num;
            }
            else if (num == 0)
            {
                // 对端关闭才会读到0
                return total;
            }
            else
            {
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                    break;// 对端数据读完了(正常情况), 退出
                else if (errno == EINTR)
                    continue;// 被信号中断, 回来继续
                perror("read");
                return -1;
            }
        }
        return total;
    }

    int writen(int fd, const char *buf, size_t size)
    {
        int total = 0;
        int num = 0;
        while (size > 0)
        {
            num = write(fd, buf, size);
            if (num > 0)
            {
                size -= num;
                total += num;
            }
            else
            {
                if (errno == EINTR)
                    continue;
                perror("write");
                return -1;
            }
        }
        return total;
    }

    std::uint64_t current_time()
    {
        std::uint64_t ret = -1;
        struct timeval tv;
        if (-1 == gettimeofday(&tv, nullptr))
            perror("gettimeofday");
        else
            ret = tv.tv_sec * (std::uint64_t) 1000 + tv.tv_usec / (std::uint64_t) 1000;
        return ret;
    }

    pid_t getThreadID()
    {
        return (pid_t)syscall(SYS_gettid);
    }

    static int _mkdir(const char* dirname)
    {
        if(access(dirname, F_OK) == 0)
            return 0;
        return mkdir(dirname, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    }

    bool createDir(const std::string& filePath)
    {
        /// mkdir函数只能在已存在的目录下创建目录, 如/xxx存在, 则创建/xxx/yyy是ok的,但是创建/xxx/yyy/zzz是不行的, 所以一层一层创建.
        std::string buf;
        for(char ch : filePath)
        {
            if(ch == '/')
            {
                if(-1 == _mkdir(buf.c_str()))
                    return false;
            }
            buf.push_back(ch);
        }
        if(-1 == _mkdir(buf.c_str()))
            return false;
        return true;
    }

    std::string getDir(const std::string& filePath)
    {
        if(filePath.empty())
            return ".";
        auto pos = filePath.rfind('/');
        if(pos == std::string::npos)
        {// 没找到'/', 表明就是当前目录
            return ".";
        }
        else if (pos == 0)
        {// 说明路径是根目录下
            return "/";
        }
        else
        {
            return filePath.substr(0, pos + 1);
        }
    }
}