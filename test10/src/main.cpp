#include <soci/soci.h>
#include <soci/postgresql/soci-postgresql.h>
#include <iostream>
#include <string>
#include <vector>
#include "CConfig.h"

int main()
{
    // 读取配置
    auto &config = CConfig::GetInstance();
    std::string configPath = "../config/CConfig.yaml";
    if (!config.Load(configPath))
    {
        std::cerr << "警告: " << config.GetLastError() << std::endl;
        std::cerr << "将使用默认配置运行" << std::endl;
    }

    // 数据库连接信息读取
    std::string dbname = config.GetStringDefault("dbname", "");
    std::string dbuser = config.GetStringDefault("user", "");
    std::string dbpass = config.GetStringDefault("password", "");
    std::string hostaddr = config.GetStringDefault("hostaddr", "127.0.0.1");
    int dbport = config.GetIntDefault("port", 5432);

    // 构建连接字符串
    if (dbname.empty() || dbuser.empty() || dbpass.empty() || hostaddr.empty() || dbport <= 0)
    {
        std::cerr << "错误: 数据库连接配置不完整。" << std::endl;
        std::cerr << "  dbname: " << (dbname.empty() ? "空" : dbname) << std::endl;
        std::cerr << "  user: " << (dbuser.empty() ? "空" : dbuser) << std::endl;
        std::cerr << "  password: " << (dbpass.empty() ? "空" : "已设置") << std::endl;
        std::cerr << "  hostaddr: " << hostaddr << std::endl;
        std::cerr << "  port: " << dbport << std::endl;
        return 2;
    }

    std::stringstream conn_ss;
    conn_ss << " dbname=" << dbname
            << " user=" << dbuser
            << " password=" << dbpass
            << " hostaddr=" << hostaddr
            << " port=" << dbport;

    std::string db_conn_str = conn_ss.str();

    // 创建数据库会话
    soci::session sql(soci::postgresql, db_conn_str);
    std::cout << "数据库连接成功!" << std::endl;

    // 示例1: 查询单个值
    std::cout << "\n1. 查询单个值:" << std::endl;
    {
        int count;
        sql << "SELECT COUNT(*) FROM users", soci::into(count);
        std::cout << "用户总数: " << count << std::endl;
    }

    // 示例2: 查询单行数据
    std::cout << "\n2. 查询单行数据:" << std::endl;
    {
        int id;
        std::string username, email;
        sql << "SELECT id, username, email FROM users WHERE id = 1",
            soci::into(id), soci::into(username), soci::into(email);

        std::cout << "用户信息 - ID: " << id
                  << ", 用户名: " << username
                  << ", 邮箱: " << email << std::endl;
    }
}