#include "log/format.h"

#include <iostream>
#include <sstream>
#include <map>
#include <functional>
#include <cctype>
#include <time.h>

void DateTimeFormatItem::format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event)
{
    time_t time = event->getTime();
    struct tm t;
    localtime_r(&time, &t);
    char buf[64];
    strftime(buf, sizeof(buf), m_format.c_str(), &t);
    os << buf;
}

LogFormatter::LogFormatter(const std::string& pattern)
    :m_pattern(pattern)
{
    init();
}

std::string LogFormatter::format(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event)
{
    std::stringstream ss;
    for(auto& item : m_vctItems)
    {
        item->format(ss, logger, level, event);
    }
    return ss.str();
}

std::ostream& LogFormatter::format(std::ostream& ofs, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event)
{
    for(auto& item : m_vctItems)
    {
        item->format(ofs, logger, level, event);
    }
    return ofs;
}

void LogFormatter::init()
{
    // 元组（str，format，type）数组
    std::vector<std::tuple<std::string,std::string, int>> vec;
    std::string nstr;
    for(size_t i = 0; i < m_pattern.size(); ++i)
    {
        if(m_pattern[i] != '%')
        {
            nstr.append(1, m_pattern[i]);
            continue;
        }

        // 如果最后一个字符是%，那就不是转义
        if((i + 1) < m_pattern.size()) 
        {
            if(m_pattern[i + 1] == '%') 
            {
                nstr.append(1, '%');
                continue;
            }
        }

        int fmt_status = 0;
        size_t fmt_begin = 0;

        std::string str;
        std::string fmt;
        size_t n = i + 1;
        // 解读{}子格式，主要%d（时间）可以指定子格式
        while(n < m_pattern.size()) {
            if(!fmt_status && (!isalpha(m_pattern[n]) && m_pattern[n] != '{'
                    && m_pattern[n] != '}')) {
                str = m_pattern.substr(i + 1, n - i - 1);
                break;
            }
            if(fmt_status == 0) {
                if(m_pattern[n] == '{') {
                    str = m_pattern.substr(i + 1, n - i - 1);
                    //std::cout << "*" << str << std::endl;
                    fmt_status = 1; //解析格式
                    fmt_begin = n;
                    ++n;
                    continue;
                }
            } else if(fmt_status == 1) {
                if(m_pattern[n] == '}') {
                    fmt = m_pattern.substr(fmt_begin + 1, n - fmt_begin - 1);
                    //std::cout << "#" << fmt << std::endl;
                    fmt_status = 0;
                    ++n;
                    break;
                }
            }
            ++n;
            if(n == m_pattern.size()) {
                if(str.empty()) {
                    str = m_pattern.substr(i + 1);
                }
            }
        }

        if(fmt_status == 0) 
        {
            if(!nstr.empty()) {
                vec.push_back(std::make_tuple(nstr, std::string(), 0));
                nstr.clear();
            }
            vec.push_back(std::make_tuple(str, fmt, 1));
            i = n - 1;
        } 
        else if(fmt_status == 1) 
        {
            std::cout << "pattern parse error: " << m_pattern << " - " << m_pattern.substr(i) << std::endl;
            m_error = true;
            vec.push_back(std::make_tuple("<<pattern_error>>", fmt, 0));
        }
    }
    if(!nstr.empty()) {
        vec.push_back(std::make_tuple(nstr, "", 0));
    }
    static std::map<std::string, std::function<LogFormatItem::ptr(const std::string& str)> > s_format_items = {
#define XX(str, C) \
        {#str, [](const std::string& fmt) { return LogFormatItem::ptr(new C(fmt));}}

        XX(m, MessageFormatItem),           //m:消息
        XX(p, LevelFormatItem),             //p:日志级别
        XX(r, ElapseFormatItem),            //r:累计毫秒数
        XX(c, NameFormatItem),              //c:日志名称
        XX(t, ThreadIdFormatItem),          //t:线程id
        XX(n, NewLineFormatItem),           //n:换行
        XX(d, DateTimeFormatItem),          //d:时间
        XX(f, FilenameFormatItem),          //f:文件名
        XX(l, LineFormatItem),              //l:行号
        XX(T, TabFormatItem),               //T:Tab
        XX(N, ThreadNameFormatItem),        //N:线程名称
#undef XX
    };

    for(auto& i : vec) 
    {
        if(std::get<2>(i) == 0) 
        {
            m_vctItems.push_back(LogFormatItem::ptr(new StringFormatItem(std::get<0>(i))));
        } 
        else 
        {
            auto it = s_format_items.find(std::get<0>(i));
            if(it == s_format_items.end())
            {
                m_vctItems.push_back(LogFormatItem::ptr(new StringFormatItem("<<error_format %" + std::get<0>(i) + ">>")));
                m_error = true;
            } 
            else
            {
                m_vctItems.push_back(it->second(std::get<1>(i)));
            }
        }
    }
}
