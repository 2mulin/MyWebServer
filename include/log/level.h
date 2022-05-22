#ifndef LOG_LEVEL_H
#define LOG_LEVEL_H

#include <string>

/**
 * @brief 日志级别
 */
class LogLevel
{
public:
    enum Level
    {
        UNKNOWN = 0,
        DEBUG = 1,
        INFO = 2,
        WARN = 3,
        ERROR = 4,
        FATAL = 5
    };

    /**
     * @brief 日志级别转化为文本字符串
     */
    static std::string ToString(LogLevel::Level level);

    /**
     * @brief 将文本字符串转化为相应日志级别
     */
    static LogLevel::Level FromString(const std::string& str);
};

#endif //LOG_LEVEL_H
