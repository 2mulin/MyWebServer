/**
 * @brief 针对于conf模块的测试。
 */

#include "conf/conf.h"
#include "conf/lexicalcast.h"

#include <iostream>
#include <cassert>

int main()
{
    ConfigManager& configManager = Singleton<ConfigManager>::getInstance();

    /// 这些接口会用该默认值记录这些配置项。
    ConfigItem<short>::ptr port = configManager.lookup<short>("webserver.port", 6666, "端口号");
    ConfigItem<int>::ptr thread_count = configManager.lookup<int>("webserver.thread_count", 4, "线程数");
    ConfigItem<std::string>::ptr htdocs = configManager.lookup<std::string>("webserver.htdocs", "/home/test", "静态web文件");

    auto task = [](short old_value, short new_value){
        std::cout << "modify port config: old_value=" << old_value << ",new_value=" << new_value << std::endl;
    };

    uint64_t cb_id = port->addCallBack(task);

    assert(port->getValue() == 6666);
    assert(thread_count->getValue() == 4);
    assert(htdocs->getValue() == "/home/test");

    /// 从配置文件中重新读取这些配置。
    configManager.loadFromYaml(YAML::LoadFile("../conf/config.yml"));

    port = configManager.lookup<short>("webserver.port");
    thread_count = configManager.lookup<int>("webserver.thread_count");
    htdocs = configManager.lookup<std::string>("webserver.htdocs");

    std::cout << port->getValue() << std::endl;
    std::cout << thread_count->getValue() << std::endl;
    std::cout << htdocs->getValue() << std::endl;

    port->delCallBack(cb_id);
    return 0;
}
