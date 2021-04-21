/***********************************************************
 *@author RedDragon
 *@date 2021/4/21
 *@brief 
***********************************************************/

#include "Log.h"
#include <ctime>
#include <pthread.h>
#include <iostream>

// LogLevel实现
std::string LogLevel::ToString(LogLevel::Level level)
{
    std::string ret = "UNKNOWN";
    switch (level)
    {
        case DEBUG:
            ret = "DEBUG";
            break;
        case INFO:
            ret = "INFO";
            break;
        case WARN:
            ret = "WARN";
            break;
        case ERROR:
            ret = "ERROR";
            break;
        case FATAL:
            ret = "FATAL";
            break;
        default:
            break;
    }
    return ret;
}

// LogInfo实现

LogInfo::LogInfo(const char* fileName, uint32_t line, const char* content)
        :m_fileName(fileName), m_line(line), m_content(content), m_pattern("%L[%d{%Y-%m-%d %H:%M:%S}]%T%t%T%f%T%l%T%m")
{}

std::string LogInfo::format()
{
    // 默认格式 "%L[%d{%Y-%m-%d %H:%M:%S}]%T%t%T%f%T%l%T%m"  级别 时间 线程id 文件名 行号 消息
    std::string ret;
    for(size_t i = 0; i < m_pattern.size(); ++i)
    {
        if(m_pattern[i] == '%')
        {
            switch (m_pattern[++i])
            {
                case 'L':
                {
                    ret += LogLevel::ToString(m_level);
                    ret += ": ";
                    break;
                }
                case 'd':
                {// 默认{%Y-%m-%d %H:%M:%S}格式
                    size_t low = m_pattern.find('{', i);
                    size_t high = m_pattern.find('}', i);
                    if (low == std::string::npos || high == std::string::npos || low > high)
                        break;  // 时间格式错误, 不处理, 直接打印出来错误格式
                    i = high;
                    std::string fmtStr = m_pattern.substr(low + 1, high - low - 1);// 时间子格式
                    time_t m_t = time(nullptr);
                    struct tm *m_tm = gmtime(&m_t);
                    char buf[1024] = {0};
                    strftime(buf, sizeof(buf) - 1, fmtStr.c_str(), m_tm);
                    ret += buf;
                    break;
                }
                case 't':
                {
                    pthread_t pid = pthread_self();
                    ret += ("ThreadID:" + std::to_string(pid));
                    break;
                }
                case 'f':
                    if(m_fileName)
                        ret += m_fileName;
                    break;
                case 'l':
                    if(m_line)
                        ret += ':' + std::to_string(m_line);
                    break;
                case 'm':
                    ret += m_content;
                    break;
                case 'n':
                    ret.push_back('\n');
                    break;
                case 'T':
                    ret.push_back('\t');
                    break;
                default:
                    ret.push_back(m_pattern[i]);
                    break;
            }
        }
        else
            ret.push_back(m_pattern[i]);
    }
    return ret;
}

// StdOutAppender实现
void StdOutLogAppender::log(LogLevel::Level level, std::string logStr)
{
    if(level >= m_level)
        std::cout << logStr;
    std::cout.flush();
}

// FileLogAppender实现
FileLogAppender::FileLogAppender(const std::string &fileName, LogLevel::Level level)
        : LogAppender(level), m_fileName(fileName)
{
    m_fileStream.open(m_fileName, std::ios::app);
}

FileLogAppender::~FileLogAppender()
{
    if(m_fileStream.is_open())
        m_fileStream.close();
}

void FileLogAppender::log(LogLevel::Level level, std::string logStr)
{
    if(level >= m_level)
        m_fileStream << logStr;
    m_fileStream.flush();
}

// logger实现

void Logger::addAppender(LogAppender::ptr appender)
{
    m_appenders.push_back(appender);
}

void Logger::delAppender(LogAppender::ptr appender)
{
    for(auto it = m_appenders.begin(); it != m_appenders.end(); ++it)
    {
        if(*it == appender)
        {
            m_appenders.erase(it);
            break;
        }
    }
}

void Logger::log(LogLevel::Level level, LogInfo::ptr logInfo)
{
    // 将要打印的内容格式化
    std::string logStr = logInfo->format();
    for(auto &it : m_appenders)
        it->log(level, logStr);
}

void Logger::debug(LogInfo::ptr logInfo)
{
    logInfo->setLevel(LogLevel::DEBUG);
    log(LogLevel::DEBUG, logInfo);
}
void Logger::info(LogInfo::ptr logInfo)
{
    logInfo->setLevel(LogLevel::INFO);
    log(LogLevel::INFO, logInfo);
}
void Logger::warn(LogInfo::ptr logInfo)
{
    logInfo->setLevel(LogLevel::WARN);
    log(LogLevel::WARN, logInfo);
}
void Logger::error(LogInfo::ptr logInfo)
{
    logInfo->setLevel(LogLevel::ERROR);
    log(LogLevel::ERROR, logInfo);
}
void Logger::fatal(LogInfo::ptr logInfo)
{
    logInfo->setLevel(LogLevel::FATAL);
    log(LogLevel::FATAL, logInfo);
    // 出现fatal, 直接退出程序
    exit(-1);
}