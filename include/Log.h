/***********************************************************
 *@author RedDragon
 *@date 2021/5/25
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

/**
 * @brief 日志级别
 */
class LogLevel
{
public:
    /// 级别
    enum Level
    {
        DEBUG = 1,
        INFO = 2,
        WARN = 3,
        ERROR = 4,
        FATAL = 5
    };
    /**
     * @brief 日志级别转化为相应字符串
     */
    static std::string ToString(LogLevel::Level level);
};

/**
 * @brief 日志详细信息
 */
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
    /// 日志级别
    LogLevel::Level m_level;
    /// 文件名
    const char* m_fileName = nullptr;
    /// 行号
    uint32_t m_line = 0;
    /// 详细日志消息
    const char* m_content;
    /// 打印日志的格式
    std::string m_pattern;
};

/**
 * @brief 日志输出地址(抽象类)
 */
class LogAppender
{
public:
    typedef std::shared_ptr<LogAppender> ptr;
    explicit LogAppender(LogLevel::Level level = LogLevel::Level::DEBUG)
            :m_level(level){}
    virtual ~LogAppender()=default;

    /**
     * @brief           纯虚函数, 真正执行写日志的地方
     * @param level     这条日志的级别
     * @param logStr    格式化后的日志
     */
    virtual void log(LogLevel::Level level, std::string logStr) = 0;

protected:
    /// 可以给定义一个m_level, 小于m_level的日志, 该输出地不写
    LogLevel::Level m_level;
};

/**
 * @brief 输出到控制台
 */
class StdOutLogAppender : public LogAppender
{
public:
    typedef std::shared_ptr<StdOutLogAppender> ptr;

    /**
     * @brief           构造函数
     * @param level     指定父类m_level的值
     */
    explicit StdOutLogAppender(LogLevel::Level level = LogLevel::Level::DEBUG)
            : LogAppender(level){}
    ~StdOutLogAppender()override = default;
    void log(LogLevel::Level level, std::string logStr) override;
};

/**
 * @brief 输出到指定文件
 */
class FileLogAppender : public LogAppender
{
public:
    typedef std::shared_ptr<FileLogAppender> ptr;

    /**
     * @brief 构造函数
     * @param fileName  输出文件
     * @param level     父类m_level的值
     */
    explicit FileLogAppender(const std::string& fileName, LogLevel::Level level = LogLevel::Level::DEBUG);
    ~FileLogAppender() override;
    void log(LogLevel::Level level, std::string logStr) override;
private:
    /// 目的文件名
    std::string m_fileName;
    std::ofstream m_fileStream;
};

/**
 * @brief 日志器
 */
class Logger
{
public:
    typedef std::shared_ptr<Logger> ptr;

    void addAppender(LogAppender::ptr appender);
    void delAppender(LogAppender::ptr appender);

    void debug(LogInfo::ptr logStr);
    void info(LogInfo::ptr logStr);
    void warn(LogInfo::ptr logStr);
    void error(LogInfo::ptr logStr);
    void fatal(LogInfo::ptr logStr);

private:
    /// 子函数
    void log(LogLevel::Level level, LogInfo::ptr logInfo);
    /// 集合(输出地集合)
    std::list<LogAppender::ptr> m_appenders;
};

#endif //WEBSERVER_LOG_H
