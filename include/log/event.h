#ifndef LOG_EVENT_H
#define LOG_EVENT_H

#include <string>
#include <sstream>
#include <memory>

#include "log/level.h"

class Logger;

/**
 * @brief 写一条日志也算是一个Event，由于该类构造函数参数非常多，所以后续定义宏来简化构造过程。
 */
class LogEvent
{
public:
    typedef std::shared_ptr<LogEvent> ptr;
    /**
     * @brief Construct a new Log Event object
     * @param[in] logger 日志器
     * @param[in] level 日志级别
     * @param[in] file 所在文件名
     * @param[in] line 所在行号
     * @param[in] threadID 线程ID
     * @param[in] time 时间戳
     * @param[in] threadName 线程名
     */
    LogEvent(std::shared_ptr<Logger> logger, LogLevel::Level level,
        const char* file, uint32_t line, uint32_t threadID,
        uint64_t time, const std::string& threadName);

    const char* getFile()const
    {
        return m_file;
    }

    uint32_t getLine()const
    {
        return m_line;
    }

    uint32_t getThreadID()const
    {
        return m_threadId;
    }

    uint64_t getTime()const
    {
        return m_time;
    }

    const std::string& getThreadName()
    {
        return m_threadName;
    }

    std::shared_ptr<Logger> getLogger()
    {
        return m_logger;
    }
    
    std::stringstream& getSS()
    {
        return m_ss;
    }

    /**
     * @brief 返回日志内容
     */
    std::string getContent()const
    {
        return m_ss.str();
    }

    LogLevel::Level getLevel()const
    {
        return m_level;
    }

    /**
     * @brief 格式化写入日志内容
     */
    void format(const char* fmt, ...);

    /**
     * @brief 格式化写入日志内容
     */
    void format(const char* fmt, va_list al);

private:
    std::shared_ptr<Logger>     m_logger;           /// 日志器
    LogLevel::Level             m_level;            /// 日志等级
    const char*                 m_file = nullptr;   /// 当前代码所在文件
    uint32_t                    m_line = 0;         /// 行号
    uint32_t                    m_threadId = 0;     /// 线程ID
    uint64_t                    m_time = 0;         /// 时间戳
    std::string                 m_threadName;       /// 线程名称
    std::stringstream           m_ss;               /// 日志内容流
};

#endif // LOG_EVENT_H