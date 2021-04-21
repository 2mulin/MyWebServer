/***********************************************************
 *@author RedDragon
 *@date 2021/4/21
 *@brief 日志系统
***********************************************************/

#ifndef WEBSERVER_LOG_H
#define WEBSERVER_LOG_H

#include <string>
#include <cstdint>
#include <memory>
#include <list>
#include <fstream>
#include <sstream>

// 日志级别
class LogLevel
{
public:
    // 级别
    enum Level
    {
        DEBUG = 1,
        INFO = 2,
        WARN = 3,
        ERROR = 4,
        FATAL = 5
    };
    /**
     * @brief 日志级别转化为字符串输出
     */
    static std::string ToString(LogLevel::Level level);
};

// 详细消息
class LogInfo
{
public:
    typedef std::shared_ptr<LogInfo> ptr;
    LogInfo(const char* fileName, uint32_t line, const char* content);
    LogInfo(const char* content)
            :LogInfo(nullptr, 0, content){}

    void setFormat(std::string pattern){m_pattern = pattern;}
    void setContent(const char* content){m_content = content;}
    void setLevel(LogLevel::Level level){m_level = level;}

    /**
     * @brief 根据m_pattern的格式, 返回格式化后的日志信息
     *  %L 日志级别
     *  %m 消息
     *  %t 线程id
     *  %d 时间
     *  %f 文件名
     *  %l 行号
     *  %T 制表符
     *  %n 换行
     */
    std::string format();

private:
    LogLevel::Level m_level;            // 日志级别
    const char* m_fileName = nullptr;   // 文件名
    uint32_t m_line = 0;                // 行号
    const char* m_content;              // 详细日志消息

    std::string m_pattern;              // 打印日志的格式
};

// 日志输出地址(抽象类)
class LogAppender
{
public:
    typedef std::shared_ptr<LogAppender> ptr;
    explicit LogAppender(LogLevel::Level level = LogLevel::Level::DEBUG)
            :m_level(level){}
    virtual ~LogAppender()=default;

    //纯虚函数, 真正执行写日志的地方
    virtual void log(LogLevel::Level level, std::string logStr) = 0;

protected:
    LogLevel::Level m_level;        // 可以给定义一个level, 小于level级别日志, 就不写了
};

// 输出到控制台
class StdOutLogAppender : public LogAppender
{
public:
    StdOutLogAppender() = default;
    StdOutLogAppender(LogLevel::Level level = LogLevel::Level::DEBUG)
            : LogAppender(level){}
    void log(LogLevel::Level level, std::string logStr) override;
};

// 输出到文件
class FileLogAppender : public LogAppender
{
public:
    FileLogAppender(const std::string& fileName, LogLevel::Level level = LogLevel::Level::DEBUG);
    ~FileLogAppender() override;
    void log(LogLevel::Level level, std::string logStr) override;
private:
    std::string m_fileName;             // 目的文件名
    std::ofstream m_fileStream;
};

// 日志器
class Logger
{
public:
    typedef std::shared_ptr<Logger> ptr;

    void addAppender(LogAppender::ptr appender);
    void delAppender(LogAppender::ptr appender);

    // 打印日志调用的函数(相应级别)
    void debug(LogInfo::ptr logStr);
    void info(LogInfo::ptr logStr);
    void warn(LogInfo::ptr logStr);
    void error(LogInfo::ptr logStr);
    void fatal(LogInfo::ptr logStr);

private:
    void log(LogLevel::Level level, LogInfo::ptr logInfo);  // 子函数
    std::list<LogAppender::ptr> m_appenders;                // 集合(输出地集合),输出可能不止一个地方
};

#endif //WEBSERVER_LOG_H
