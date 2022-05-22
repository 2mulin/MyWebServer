#include "log/level.h"

std::string LogLevel::ToString(LogLevel::Level level)
{
    std::string str = "UNKNOWN";
    switch (level)
    {
        case DEBUG:
            str = "DEBUG";
            break;
        case INFO:
            str = "INFO";
            break;
        case WARN:
            str = "WARN";
            break;
        case ERROR:
            str = "ERROR";
            break;
        case FATAL:
            str = "FATAL";
            break;
        default:
            break;
    }
    return str;
}

LogLevel::Level LogLevel::FromString(const std::string& str)
{
#define XX(level, s)    \
    if(str == #s)       \
        return LogLevel::Level::level;

    XX(DEBUG, debug);
    XX(DEBUG, DEBUG);
    XX(INFO, info);
    XX(INFO, INFO);
    XX(WARN, warn);
    XX(WARN, WARN);
    XX(ERROR, error);
    XX(ERROR, ERROR);
    XX(FATAL, fatal);
    XX(FATAL, FATAL);
#undef XX
    return LogLevel::Level::UNKNOWN;
}