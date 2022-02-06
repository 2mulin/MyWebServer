#ifndef LOG_FORMAT_H
#define LOG_FORMAT_H

#include <string>
#include <vector>
#include <memory>

#include "log/level.h"
#include "log/event.h"

/**
 * @brief 日志格式化内容项(基类)
 */
class LogFormatItem
{
public:
    typedef std::shared_ptr<LogFormatItem> ptr;
    /**
     * @brief 析构函数
     */
    virtual ~LogFormatItem() {}
    /**
     * @brief 格式化日志到流
     * @param[in, out] os 日志输出流
     * @param[in] logger 日志器
     * @param[in] level 日志等级
     * @param[in] event 日志事件
     */
    virtual void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) = 0;
};

/**
 * @brief 消息内容项
 */
class MessageFormatItem : public LogFormatItem
{
public:
    MessageFormatItem(const std::string& str = "")
    {}

    void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override
    {
        os << event->getContent();
    }
};

/**
 * @brief 日志等级项
 */
class LevelFormatItem : public LogFormatItem 
{
public:
    LevelFormatItem(const std::string& str = "")
    {}

    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override 
    {
        os << LogLevel::ToString(level);
    }
};

/**
 * @brief 程序持续运行时间项
 */
class ElapseFormatItem : public LogFormatItem 
{
public:
    ElapseFormatItem(const std::string& str = "")
    {}

    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override 
    {
        os << event->getElapse();
    }
};

/**
 * @brief 日志器名称
 */
class NameFormatItem : public LogFormatItem 
{
public:
    NameFormatItem(const std::string& str = "") 
    {}
    
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override 
    {
        // logger还没实现
        //os << event->getLogger()->getName();
    }
};

/**
 * @brief 日志器名称项
 */
class ThreadIdFormatItem : public LogFormatItem 
{
public:
    ThreadIdFormatItem(const std::string& str = "")
    {}

    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override 
    {
        os << event->getThreadID();
    }
};

/**
 * @brief 线程名
 */
class ThreadNameFormatItem : public LogFormatItem
{
public:
    ThreadNameFormatItem(const std::string& str = "") 
    {}

    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override 
    {
        os << event->getThreadName();
    }
};

/**
 * @brief 换行符项
 */
class NewLineFormatItem : public LogFormatItem 
{
public:
    NewLineFormatItem(const std::string& str = "")
    {}

    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override 
    {
        os << std::endl;
    }
};

/**
 * @brief 行号项
 */
class LineFormatItem : public LogFormatItem 
{
public:
    LineFormatItem(const std::string& str = "") 
    {}

    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override 
    {
        os << event->getLine();
    }
};

/**
 * @brief 文件名项
 */
class FilenameFormatItem : public LogFormatItem 
{
public:
    FilenameFormatItem(const std::string& str = "") 
    {}

    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override 
    {
        os << event->getFile();
    }
};

/**
 * @brief Tab键项
 */
class TabFormatItem : public LogFormatItem
{
public:
    TabFormatItem(const std::string& str = "")
    {}

    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override 
    {
        os << '\t';
    }
};

/**
 * @brief 时间项，内部调用strftime获取。
 */
class DateTimeFormatItem : public LogFormatItem
{
public:
    DateTimeFormatItem(const std::string& format = "%Y-%m-%d %H:%M:%S")
        :m_format(format)
    {
        // 防止用户传进来的format为空
        if(m_format.empty())
            m_format = "%Y-%m-%d %H:%M:%S";
    }

    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override;

private:
    std::string m_format;
};

/**
 * @brief 字符串项，format原封不动，流输入string
 */
class StringFormatItem : public LogFormatItem 
{
public:
    StringFormatItem(const std::string& str)
        :m_string(str) {}
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override 
    {
        os << m_string;
    }

private:
    std::string m_string;
};

/**
 * @brief 将LogEvent内容格式化，转化成string
 */
class LogFormatter
{
public:
    typedef std::shared_ptr<LogFormatter> ptr;
    /**
     * @brief 构造函数
     * @param[in] pattern 格式模板
     * @details 
     *  %m 消息内容
     *  %p 日志级别
     *  %r 累计毫秒数
     *  %c 日志名称
     *  %t 线程id
     *  %n 换行
     *  %d 时间,后面加{}指定具体格式
     *  %f 文件名
     *  %l 行号
     *  %T 制表符
     *  %N 线程名称
     *
     *  默认格式 "%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T[%p]%T[%c]%T%f:%l%T%m%n"
     */
    LogFormatter(const std::string& pattern);

    std::string format(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event);
    std::ostream& format(std::ostream& ofs, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event);

    /**
     * @brief 初始化，解析日志模板（重点）
     */
    void init();

    /**
     * @brief 返回日志格式
     */
    const std::string getPattern() const
    {
        return m_pattern;
    }

    /**
     * @brief 是否有错误
     */
    bool isError() const
    {
        return m_error;
    }

private:
    /// 日志格式模板
    std::string m_pattern;
    /// 日志格式解析后格式
    std::vector<LogFormatItem::ptr> m_vctItems;
    /// 是否有错误
    bool m_error = false;
};

#endif // LOG_FORMAT_H
