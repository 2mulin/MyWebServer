#include "conf.h"

#include <fstream>
#include <iostream>

#include "log/log.h"

static Logger::ptr logger = std::make_shared<Logger>("Conf_Logger");

Conf::Conf(std::string fileName)
        :m_fileName(fileName)
{
    
    int ret = readConf("../conf/server.conf");
    if(ret == 0)
        LOG_INFO(logger) << "配置文件读取完毕!" << std::endl;
    else if(ret == -1)
    {
        LOG_INFO(logger) << "配置文件无法打开!" << std::endl;
        exit(-1);
    }
    else
    {
        LOG_FATAL(logger) << std::to_string(ret) + "行格式错误!" << std::endl;
        exit(-1);
    }
}

int Conf::readConf(std::string pathName)
{
    std::ifstream buf(pathName);
    if(!buf.is_open())
        return -1;
    char str[1024] = {0};
    int line = 1;
    while(buf)
    {
        // 读取一行
        buf.getline(str,1024,'\n');
        if(str[0] == '#' || str[0] == '\0')
            continue;// 空行 或者 该行被注释了

        bool keyOrVal = false;
        int pos = 0;
        std::string key, value;
        while(str[pos] != '\0')
        {
            if(str[pos] == ':' && str[pos+1] == ' ')
            {
                keyOrVal = true;
                pos += 2;
                continue;
            }
            if(keyOrVal)
                value.push_back(str[pos]);
            else
                key.push_back(str[pos]);
            ++pos;
        }
        // 读完一行, key或这value为空,表示格式错误.
        if(key.empty() || value.empty())
            return line;
        m_conf.insert(std::pair<std::string, std::string>{key, value});
        ++line;
    }
    return 0;
}

uint16_t Conf::getPort() const
{
    auto item = m_conf.find("PORT");
    int val = std::stoi(item->second);
    if(val < 1024 || val > 65535)
    {
        LOG_FATAL(logger) << "端口号大小设置错误!" << std::endl;
        exit(-1);
    }
    return val;
}

uint16_t Conf::getMaxThreadCount() const
{
    auto item = m_conf.find("MaxThreadCount");
    uint16_t val = std::stoi(item->second);
    return val;
}

std::string Conf::getHtdocs() const
{
    return m_conf.find("htdocs")->second;
}