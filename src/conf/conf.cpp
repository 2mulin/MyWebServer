#include "conf/conf.h"

#include <unistd.h>
#include <getopt.h>

static void listAllMember(const std::string& prefix, const YAML::Node& node,
                          std::list<std::pair<std::string, const YAML::Node> >& output)
{
    /// 如果出现了"abcdefghijklmnopqrstuvwxyz._0123456789"字符之外的字符, 说明配置名格式不正确
    if(prefix.find_first_not_of("abcdefghijklmnopqrstuvwxyz._0123456789")
       != std::string::npos)
    {
        LOG_ERROR(LOG_ROOT()) << "ConfigItem invalid name: " << prefix << " : " << node;
        return;
    }
    output.push_back(std::make_pair(prefix, node));
    if(node.IsMap()) {
        for(auto it = node.begin(); it != node.end(); ++it) {
            // 这个scalar表示yaml纯量, 最基本,不可再分的值.
            listAllMember(prefix.empty() ? it->first.Scalar()
                                         : prefix + "." + it->first.Scalar(), it->second, output);
        }
    }
}


ConfigItemBase::ptr ConfigManager::lookupBase(const std::string &name)
{
    WebServer::ScopedLock<WebServer::Mutex> lk(m_mtx);
    auto it = m_mapConfigs.find(name);
    if(it != m_mapConfigs.end())
        return it->second;
    return nullptr;
}

void ConfigManager::loadFromYaml(const YAML::Node& root)
{
    std::list<std::pair<std::string, const YAML::Node> > allNodes;
    listAllMember("", root, allNodes);

    for(auto& i : allNodes)
    {
        std::string key = i.first;
        if(key.empty())
            continue;

        // std::transform将一个函数应用于某范围的各个元素，并在目标范围存储结果。这里是将key中的字符全部转化为小写
        std::transform(key.begin(), key.end(), key.begin(), ::tolower);
        ConfigItemBase::ptr item = lookupBase(key);

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
        else
        {
            if(i.second.IsScalar())
                LOG_WARN(LOG_ROOT()) << "{" << key << "} exists Invalid configuration item!";
        }
    }
}

bool ConfigManager::loadFromCmd(int argc, char **argv)
{
    static struct option long_options[] = {
        {"port", required_argument, NULL, 1},
        {"threadcount", required_argument, NULL, 2}
    };
    int opt = -1;
    int option_index = 0;
    while((opt = getopt_long(argc, argv, "p:t:", long_options, &option_index)) != -1)
    {
        if(opt == 'p' || opt == 1)
        {
            ConfigItem<uint16_t>::ptr item = lookup<uint16_t>("port", 7089);
            item->fromString(std::string(optarg));
        }
        else if(opt == 't' || opt == 2)
        {
            ConfigItem<uint32_t>::ptr item = lookup<uint32_t>("threadcount", 4);
            item->fromString(std::string(optarg));
        }
        else
        {// 处理不是option的多余输入
            LOG_ERROR(LOG_ROOT()) << "usage: ./app [-p, --port] num [-t, --threadcount] num";
            return false;
        }
    }

    // argv中，不应该存在的参数
    std::string info;
    while(optind < argc)
    {
        info += std::string(argv[optind++]);
    }
    LOG_WARN(LOG_ROOT()) << info;

    return true;
}

void ConfigManager::visit(std::function<void(ConfigItemBase::ptr)> cb)
{
    WebServer::ScopedLock<WebServer::Mutex> lk(m_mtx);
    for(auto iter = m_mapConfigs.begin(); iter != m_mapConfigs.end(); ++iter)
    {
        cb(iter->second);
    }
}
