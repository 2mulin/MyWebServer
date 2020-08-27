/***********************************************************
 *@author RedDragon
 *@date 2020/8/26
 *@brief 
***********************************************************/

#ifndef WEBSERVER_REQUESTDATA_H
#define WEBSERVER_REQUESTDATA_H

#include <string>
#include <unordered_map>

// 用到了单例模式
class MimeType{
private:
    static pthread_mutex_t lock;
    static std::unordered_map<std::string,std::string> mime;
    MimeType();
    MimeType(const MimeType &m);
public:
    static std::string getMime(const std::string &suffix);
};


#endif //WEBSERVER_REQUESTDATA_H
