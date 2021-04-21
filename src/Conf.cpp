/***********************************************************
 *@author RedDragon
 *@date 2021/4/20
 *@brief 
***********************************************************/

#include "Conf.h"
#include <fstream>
#include <iostream>

using std::ifstream;

Conf::Conf(std::string fileName)
        :m_fileName(fileName)
{
    int ret = readConf("../conf/server.conf");
    if(ret == 0)
        m_logger->info("配置读取完毕!");
    else if(ret == -1)
    {
        m_logger->fatal("配置文件无法打开!")
        exit(-1);
    }
    else
    {
        m_logger->fatal(std::to_string(ret) + "行格式错误!");
        exit(-1);
    }
}

int Conf::readConf(std::string pathName)
{
    ifstream buf(pathName);
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
        m_conf.insert(key, value);
        ++line;
    }
    return 0;
}

uint16_t Conf::getPort() const
{
    auto item = m_conf.find("PORT");
    int val = item->second;
    if(val < 1024 || val > 65535)
    {
        m_logger->fatal("端口号大小设置错误!");
        exit(-1);
    }
    return val;
}

uint16_t Conf::getMaxThreadCount() const
{
    auto item = m_conf.find("MaxThreadCount");
    uint16_t val = item->second;
    return val;
}

string Conf::getHtdocs() const
{
    return m_conf.find("htdocs");
}