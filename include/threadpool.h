/***********************************************************
 *@author RedDragon
 *@date 2020/9/5
 *@brief 
***********************************************************/

#ifndef WEBSERVER_THREADPOOL_H
#define WEBSERVER_THREADPOOL_H

#include <pthread.h>
#include "requestData.h"

typedef enum
{
    immediate_shutdown = 1,// 立即
    graceful_shutdown  = 2// 优美
}threadpool_shutdown_t;


/**************************************
 * @struct threadpool_task
 * @brief the work struct
 *
 * @var function Pointer to the function that will perform the task.
 * @var argument Argument to be passed to the function.
 *************************************/
typedef struct
{
    void (*function)(void*);
    void *argument;
} threadpool_task_t;

#endif //WEBSERVER_THREADPOOL_H
