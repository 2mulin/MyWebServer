/***********************************************************
 *@author RedDragon
 *@date 2021/4/20
 *@brief 配置系统的简单是实现
 * 直接读取conf目录下的server.conf文件.
 * 按照key: value的格式读,直接存粗到哈希表中.
***********************************************************/
#ifndef WEBSERVER_CONF_H
#define WEBSERVER_CONF_H

#include "Log.h"

#include <cstdint>
#include <string>
#include <unordered_map>
using std::string;
using std::unordered_map;

class Conf
{
private:
    Logger::ptr m_logger;

    std::string m_fileName;
    std::unordered_map<string, string> m_conf;
    /*
     * @brief: 读取指定配置文件
     * @return 0表示成功, -1表示文件无法打开, (n > 0)表示n行有格式错误.
     */
    int readConf(std::string pathName);

public:
    Conf(std::string fileName);
    /*
     * @brief: 返回设置的端口号
     */
    uint16_t getPort()const;
    uint16_t getMaxThreadCount()const;
    /*
     * @brief: 返回设置的资源目录
     */
    string getHtdocs()const;
};

#endif //WEBSERVER_CONF_H