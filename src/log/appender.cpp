#include "log/appender.h"

#include <iostream>
#include <mutex>

LogFormatter::ptr LogAppender::getFormatter()
{
    std::lock_guard<std::mutex> lock(m_mtx);
    return m_formatter;
}

void LogAppender::setFormatter(LogFormatter::ptr newFormatter)
{
    std::lock_guard<std::mutex> lock(m_mtx);
    m_formatter = newFormatter;
}

void StdOutLogAppender::log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event)
{
    if(level >= m_level)
    {
        std::lock_guard<std::mutex> lock(m_mtx);
        m_formatter->format(logger, level, event);
    }
}

FileLogAppender::FileLogAppender(const std::string &fileName)
    :m_fileName(fileName)
{
    //m_fileStream.open(m_fileName, std::ios::app);
    reopen();
}

FileLogAppender::~FileLogAppender()
{
    if(m_fileStream.is_open())
        m_fileStream.close();
}

bool FileLogAppender::reopen()
{
    // 借助util的实现。
    return true;
}

void FileLogAppender::log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event)
{
    if(level >= m_level) {
        uint64_t now = event->getTime();
        // 超过3秒就会刷新一次文件缓存（关闭文件重新打开，会刷新文件缓存）
        if(now >= (m_lastTime + 3)) {
            reopen();
            m_lastTime = now;
        }
        std::lock_guard<std::mutex> lock(m_mtx);
        if(!m_formatter->format(m_fileStream, logger, level, event)) {
            std::cout << "error" << std::endl;
        }
    }
}
