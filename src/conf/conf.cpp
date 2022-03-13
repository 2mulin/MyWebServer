#include "conf/conf.h"

/// 静态变量初始化

ConfigManager::ConfigItemMap ConfigManager::m_mapConfigs;

std::mutex ConfigManager::m_mtx;

static Logger::ptr g_logger = LOG_NAME("system");

static void ListAllMember(const std::string& prefix, const YAML::Node& node,
                          std::list<std::pair<std::string, const YAML::Node> >& output)
{
    // 如果出现了"abcdefghikjlmnopqrstuvwxyz._012345678"字符之外的字符, 表示格式不正确
    if(prefix.find_first_not_of("abcdefghikjlmnopqrstuvwxyz._012345678")
       != std::string::npos)
    {
        LOG_ERROR(g_logger) << "Config invalid name: " << prefix << " : " << node;
        return;
    }
    output.push_back(std::make_pair(prefix, node));
    if(node.IsMap()) {
        for(auto it = node.begin(); it != node.end(); ++it) {
            // 这个scalar表示纯量,就是不可再分
            ListAllMember(prefix.empty() ? it->first.Scalar()
                                         : prefix + "." + it->first.Scalar(), it->second, output);
        }
    }
}


ConfigItemBase::ptr ConfigManager::LookupBase(const std::string &name)
{
    std::lock_guard<std::mutex> lk(m_mtx);
    auto it = m_mapConfigs.find(name);
    if(it != m_mapConfigs.end())
        return it->second;
    return nullptr;
}

void ConfigManager::LoadFromYaml(const YAML::Node& root)
{
    std::list<std::pair<std::string, const YAML::Node> > allNodes;
    ListAllMember("", root, allNodes);

    for(auto& i : allNodes)
    {
        std::string key = i.first;
        if(key.empty())
            continue;

        // 将一个函数应用于某一范围的各个元素，并在目标范围存储结果, 这里是全部转化为小写
        std::transform(key.begin(), key.end(), key.begin(), ::tolower);
        ConfigItemBase::ptr item = LookupBase(key);


        /// 这一段有点看不懂呀
        if(item)
        {
            if(i.second.IsScalar())
            {
                item->fromString(i.second.Scalar());
            }
            else
            {
                std::stringstream ss;
                ss << i.second;
                item->fromString(ss.str());
            }
        }
    }
}

void ConfigManager::Visit(std::function<void(ConfigItemBase::ptr)> cb)
{
    std::lock_guard<std::mutex> lk(m_mtx);
    for(auto iter = m_mapConfigs.begin(); iter != m_mapConfigs.end(); ++iter)
    {
        cb(iter->second);
    }
}

/*
 * 还是一个LoadFromConfDir函数,用于读取指定目录的配置.
 */
