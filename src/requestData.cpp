/***********************************************************
 *@author RedDragon
 *@date 2020/8/26
 *@brief 
***********************************************************/

#include "requestData.h"

std::string MimeType::getMime(const std::string &suffix) {
    if(mime.size() == 0){
        pthread_mutex_lock(&lock);
    }
}