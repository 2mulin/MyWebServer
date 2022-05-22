/**
* @brief 简单测试log模块
*/

#include "log/log.h"
#include "util/util.h"

int main()
{
    Logger::ptr logger = std::make_shared<Logger>();
    logger->addAppender(std::make_shared<StdOutLogAppender>());

    FileLogAppender::ptr fileLogAppender = std::make_shared<FileLogAppender>("../log/test/log.txt");
    LogFormatter::ptr fmt = std::make_shared<LogFormatter>("%d%T%p%T%m%n");

    fileLogAppender->setFormatter(fmt);
    fileLogAppender->setLevel(LogLevel::INFO);

    logger->addAppender(fileLogAppender);

    LOG_DEBUG(logger) << "test LOG_DEBUG macro!";
    LOG_INFO(logger) << "test LOG_INFO macro!";
    LOG_WARN(logger) << "test LOG_WARN macro!";
    LOG_ERROR(logger) << "test LOG_ERROR macro!";
    LOG_FATAL(logger) << "test LOG_FATAL macro!";

    LOG_FMT_DEBUG(logger, "test LOG_FMT_DEBUG macro! %s", "success!");
    LOG_FMT_INFO(logger, "test LOG_FMT_INFO macro! %s", "success!");
    LOG_FMT_WARN(logger, "test LOG_FMT_WARN macro! %s", "success!");
    LOG_FMT_ERROR(logger, "test LOG_FMT_ERROR macro! %s", "success!");
    LOG_FMT_FATAL(logger, "test LOG_FMT_FATAL macro! %s", "success!");

    // xx logger 不存在,所以会创建一个, 测试创建出来的xx日志器是否正常.
    auto l = LoggerMgr::getInstance().getLogger("xx");
    LOG_DEBUG(logger) << "test LOG_DEBUG macro!";
    LOG_INFO(logger) << "test LOG_INFO macro!";
    LOG_WARN(logger) << "test LOG_WARN macro!";
    LOG_ERROR(logger) << "test LOG_ERROR macro!";
    return 0;
}
