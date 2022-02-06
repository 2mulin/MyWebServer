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
     * @param[in] elapse 程序运行到现在的时间
     * @param[in] threadID 线程ID
     * @param[in] time 时间戳
     * @param[in] threadName 线程名
     */
    LogEvent(std::shared_ptr<Logger> logger, LogLevel::Level level,
        const char* file, uint32_t line, uint32_t elapse,
        uint32_t threadID, uint64_t time, const std::string& threadName);

    const char* getFile()const
    {
        return m_file;
    }

    uint32_t getLine()const
    {
        return m_line;
    }

    uint32_t getElapse()const
    {
        return m_elapse;
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
    /// 日志器
    std::shared_ptr<Logger> m_logger;
    /// 日志等级
    LogLevel::Level m_level;
    /// 文件名
    const char* m_file = nullptr;
    /// 行号
    uint32_t m_line = 0;
    /// 程序启动开始到现在的毫秒数
    uint32_t m_elapse = 0;
    /// 线程ID
    uint32_t m_threadId = 0;
    /// 时间戳
    uint64_t m_time = 0;
    /// 线程名称
    std::string m_threadName;
    /// 日志内容流
    std::stringstream m_ss;
};

#endif // LOG_EVENT_H