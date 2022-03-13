#ifndef CONF_LEXICALCAST_H
#define CONF_LEXICALCAST_H

#include <string>
#include <vector>
#include <list>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>

#include <boost/lexical_cast.hpp>
#include "yaml-cpp/yaml.h"

/**
 * @brief 类型转化模板类
 */
template <typename F, typename T>
class LexicalCast
{
public:
    /**
     * @brief 类型转换，将F类型转化为T
     * @param[in] v 源类型值
     * @return 返回v转换后的目标类型
     * @exception 当类型不可转换时抛出异常
     */
    T operator()(const F &v)
    {
        return boost::lexical_cast<T>(v);
    }
};

/**
 * @brief  类型转化偏特化，string 转化为 std::vector<T>
 * @tparam T
 */
template <typename T>
class LexicalCast<std::string, std::vector<T>>
{
public:
    std::vector<T> operator()(const std::string &v)
    {
        YAML::Node node = YAML::Load(v);
        // 必须使用typename std::vector<T> vct; 下面这种用法应该会报错
        std::vector<T> vct;
        std::stringstream ss;
        for (size_t i = 0; i < node.size(); ++i)
        {
            ss.str(""); // 替换底层buf为""
            ss << node[i];
            vct.push_back(LexicalCast<std::string, T>()(ss.str()));
        }
        return vct;
    }
};

/**
 * @brief 类型转化偏特化，std::vector<T> 转化为 string
 * @tparam T
 */
template <typename T>
class LexicalCast<std::vector<T>, std::string>
{
public:
    std::string operator()(const std::string &v)
    {
        YAML::Node node(YAML::NodeType::Sequence);
        for (auto &i : v)
        {
            node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

/**
 * @brief 类型转化模板，string转化为std::list<T>
 * @tparam T
 */
template <typename T>
class LexicalCast<std::string, std::list<T>>
{
public:
    std::list<T> operator()(const std::string &v)
    {
        YAML::Node node = YAML::Load(v);
        std::list<T> ls;
        std::stringstream ss;
        for (size_t i = 0; i < node.size(); ++i)
        {
            ss.str("");
            ss << node[i];
            ls.push_back(LexicalCast<std::string, T>()(ss.str()));
        }
        return ls;
    }
};

/**
 * @brief 类型转换模板类片特化(std::list<T> 转换成 YAML String)
 */
template <class T>
class LexicalCast<std::list<T>, std::string>
{
public:
    std::string operator()(const std::list<T> &v)
    {
        YAML::Node node(YAML::NodeType::Sequence);
        for (auto &i : v)
        {
            node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

/**
 * @brief 类型转换模板类片特化(YAML String 转换成 std::set<T>)
 */
template <class T>
class LexicalCast<std::string, std::set<T>>
{
public:
    std::set<T> operator()(const std::string &v)
    {
        YAML::Node node = YAML::Load(v);
        typename std::set<T> result;
        std::stringstream ss;
        for (size_t i = 0; i < node.size(); ++i)
        {
            ss.str("");
            ss << node[i];
            result.insert(LexicalCast<std::string, T>()(ss.str()));
        }
        return result;
    }
};

/**
 * @brief 类型转换模板类片特化(std::set<T> 转换成 YAML String)
 */
template <class T>
class LexicalCast<std::set<T>, std::string>
{
public:
    std::string operator()(const std::set<T> &v)
    {
        YAML::Node node(YAML::NodeType::Sequence);
        for (auto &i : v)
        {
            node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

/**
 * @brief 类型转换模板类片特化(YAML String 转换成 std::unordered_set<T>)
 */
template <class T>
class LexicalCast<std::string, std::unordered_set<T>>
{
public:
    std::unordered_set<T> operator()(const std::string &v)
    {
        YAML::Node node = YAML::Load(v);
        typename std::unordered_set<T> vec;
        std::stringstream ss;
        for (size_t i = 0; i < node.size(); ++i)
        {
            ss.str("");
            ss << node[i];
            vec.insert(LexicalCast<std::string, T>()(ss.str()));
        }
        return vec;
    }
};

/**
 * @brief 类型转换模板类片特化(std::unordered_set<T> 转换成 YAML String)
 */
template <class T>
class LexicalCast<std::unordered_set<T>, std::string>
{
public:
    std::string operator()(const std::unordered_set<T> &v)
    {
        YAML::Node node(YAML::NodeType::Sequence);
        for (auto &i : v)
        {
            node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

/**
 * @brief 类型转换模板类片特化(YAML String 转换成 std::map<std::string, T>)
 */
template <class T>
class LexicalCast<std::string, std::map<std::string, T>>
{
public:
    std::map<std::string, T> operator()(const std::string &v)
    {
        YAML::Node node = YAML::Load(v);
        typename std::map<std::string, T> result;
        std::stringstream ss;
        for (auto it = node.begin(); it != node.end(); ++it)
        {
            ss.str("");
            ss << it->second;
            result.insert(std::make_pair(it->first.Scalar(),
                                         LexicalCast<std::string, T>()(ss.str())));
        }
        return result;
    }
};

/**
 * @brief 类型转换模板类片特化(std::map<std::string, T> 转换成 YAML String)
 */
template <class T>
class LexicalCast<std::map<std::string, T>, std::string>
{
public:
    std::string operator()(const std::map<std::string, T> &v)
    {
        YAML::Node node(YAML::NodeType::Map);
        for (auto &i : v)
        {
            node[i.first] = YAML::Load(LexicalCast<T, std::string>()(i.second));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

/**
 * @brief 类型转换模板类片特化(YAML String 转换成 std::unordered_map<std::string, T>)
 */
template <class T>
class LexicalCast<std::string, std::unordered_map<std::string, T>>
{
public:
    std::unordered_map<std::string, T> operator()(const std::string &v)
    {
        YAML::Node node = YAML::Load(v);
        typename std::unordered_map<std::string, T> vec;
        std::stringstream ss;
        for (auto it = node.begin();
             it != node.end(); ++it)
        {
            ss.str("");
            ss << it->second;
            vec.insert(std::make_pair(it->first.Scalar(),
                                      LexicalCast<std::string, T>()(ss.str())));
        }
        return vec;
    }
};

/**
 * @brief 类型转换模板类片特化(std::unordered_map<std::string, T> 转换成 YAML String)
 */
template <class T>
class LexicalCast<std::unordered_map<std::string, T>, std::string>
{
public:
    std::string operator()(const std::unordered_map<std::string, T> &v)
    {
        YAML::Node node(YAML::NodeType::Map);
        for (auto &i : v)
        {
            node[i.first] = YAML::Load(LexicalCast<T, std::string>()(i.second));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

#endif // ! CONF_LEXICALCAST_H