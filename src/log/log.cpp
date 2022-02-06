#include "log/log.h"
#include <ctime>
#include <mutex>

Logger::Logger(const std::string& name)
    :m_name(name), m_level(LogLevel::DEBUG) 
{
    m_formatter.reset(new LogFormatter("%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"));
}

void Logger::addAppender(LogAppender::ptr appender)
{
    std::lock_guard<std::mutex> lock(m_mtx);
    // 如果该appender的formatter为空，那就指定一个。
    if(!appender->getFormatter())
        appender->setFormatter(m_formatter);
    m_appenders.push_back(appender);
}

void Logger::delAppender(LogAppender::ptr appender)
{
    std::lock_guard<std::mutex> lock(m_mtx);
    for(auto it = m_appenders.begin(); it != m_appenders.end(); ++it)
    {
        if(*it == appender)
        {
            m_appenders.erase(it);
            break;
        }
    }
}

void Logger::clearAppenders()
{
    std::lock_guard<std::mutex> lock(m_mtx);
    m_appenders.clear();
}

/// 子函数
void Logger::log(LogLevel::Level level, LogEvent::ptr event)
{
    // 将要打印的内容格式化
    if(level >= m_level)
    {
        /// this智能指针
        auto self = shared_from_this();
        std::lock_guard<std::mutex> lock(m_mtx);
        if(!m_appenders.empty()) {
            for(auto& item : m_appenders) {
                item->log(self, level, event);
            }
        } else if(m_root) {
            m_root->log(level, event);
        }
    }
}

void Logger::debug(LogEvent::ptr event)
{
    log(LogLevel::DEBUG, event);
}
void Logger::info(LogEvent::ptr event)
{
    log(LogLevel::INFO, event);
}
void Logger::warn(LogEvent::ptr event)
{
    log(LogLevel::WARN, event);
}
void Logger::error(LogEvent::ptr event)
{
    log(LogLevel::ERROR, event);
}
void Logger::fatal(LogEvent::ptr event)
{
    log(LogLevel::FATAL, event);
}

LoggerManager::LoggerManager()
{
    m_root.reset(new Logger);
    LogAppender::ptr appender = std::make_shared<StdOutLogAppender>();
    m_root->addAppender(appender);

    // 维护map映射
    m_loggers[m_root->m_name] = m_root;
}

Logger::ptr LoggerManager::getLogger(const std::string& name)
{
    std::lock_guard<std::mutex> lock(m_mtx);
    auto it = m_loggers.find(name);
    if(it != m_loggers.end())
        return it->second;

    // 指定name的Logger不存在，就创建一个并返回。
    Logger::ptr logger(new Logger(name));
    logger->m_root = m_root;
    m_loggers[name] = logger;
    return logger;
}
