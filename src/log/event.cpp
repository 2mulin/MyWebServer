#include "log/event.h"

#include <cstdarg>
#include <cstdio>
#include <utility>

LogEvent::LogEvent(std::shared_ptr<Logger> logger, LogLevel::Level level,
        const char* file, uint32_t line, uint32_t threadID,
        uint64_t time, const std::string& threadName)
    :m_logger(std::move(logger)), m_level(level),
    m_file(file), m_line(line),m_threadId(threadID),
    m_time(time), m_threadName(threadName)
{}

void LogEvent::format(const char* fmt, ...)
{
    va_list al;
    va_start(al, fmt);
    format(fmt, al);
    va_end(al);
}

void LogEvent::format(const char* fmt, va_list al)
{
    char *buf = nullptr;
    int len = vasprintf(&buf, fmt, al); // 详见`man vasprintf`
    if(len != -1)
    {
        m_ss << std::string(buf, len);
        free(buf);
    }
}
