// subproject3/src/main.cpp
// 功能：通过 common_include/CConfig.h 读取配置，然后使用 libpqxx 从 PostgreSQL 的 persons 表读取数据并以 JSON 输出。
// 返回值：
//   0 - 成功（已打印 JSON 数组）
//   2 - 读取配置失败
//   3 - 数据库操作失败

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <nlohmann/json.hpp>
#include <pqxx/pqxx>
#include "CConfig.h"

using json = nlohmann::json;

namespace ns
{
    struct person
    {
        int id;
        std::string username;
        std::string full_name;
        std::string email;
        std::string phone;
    };

    // ADL序列化函数
    void to_json(json &j, const person &p)
    {
        j = json{{"id", p.id}, 
                 {"username", p.username}, 
                 {"full_name", p.full_name}, 
                 {"email", p.email}, 
                 {"phone", p.phone}};
    }
}

// 函数1：查询所有用户数据并输出为 JSON 数组
json query_users_as_json(const std::string& db_conn_str)
{
        // 连接数据库
        pqxx::connection conn(db_conn_str);
        
        // 开始事务
        pqxx::work txn(conn);

        // 执行查询：获取全部用户数据
        std::cout << "正在查询 users 表..." << std::endl;
        pqxx::result res = txn.exec("SELECT * FROM users");

        // 存储结果的 JSON 数组
        json j_array = json::array();

        // 遍历查询结果并填充 JSON 数组
        for (const auto &row : res) {
            ns::person p;
            p.id = row["id"].as<int>();
            p.username = row["username"].as<std::string>();
            p.full_name = row["full_name"].as<std::string>();
            p.email = row["email"].as<std::string>();
            p.phone = row["phone"].as<std::string>();

            j_array.push_back(p); // 使用 ADL 序列化
        }

        txn.commit();
        return j_array;  // 返回JSON结果
}

// 函数2：参数化查询特定用户
json query_user_by_id(const std::string& db_conn_str, int user_id) {
    try {
        // 连接数据库
        pqxx::connection conn(db_conn_str);
        
        // 开始事务
        pqxx::work txn(conn);

        // 执行参数化查询：防止SQL注入
        std::cout << "正在查询用户ID: " << user_id << std::endl;
        const std::string sql = "SELECT * FROM users WHERE id = $1";
        pqxx::result res = txn.exec(sql, pqxx::params{user_id});

        // 存储结果的 JSON 数组
        json j_array = json::array();

        // 遍历查询结果
        for (const auto &row : res) {
            ns::person p;
            p.id = row["id"].as<int>();
            p.username = row["username"].as<std::string>();
            p.full_name = row["full_name"].as<std::string>();
            p.email = row["email"].as<std::string>();
            p.phone = row["phone"].as<std::string>();

            j_array.push_back(p);
        }

        txn.commit();
        return j_array;  // 返回JSON结果
        
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("查询用户失败: ") + e.what());
    }
}

int main()
{
    std::cout << "=== 从 PostgreSQL 读取 users 表数据并输出为 JSON ===\n\n";

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

    try
    {
        // // 调用函数1：查询所有用户数据并输出为 JSON 数组
        // std::cout << "=== 示例1：查询所有用户 ===" << std::endl;
        // json j_array = query_users_as_json(db_conn_str);
        // // 处理查询结果
        // if (j_array.empty()) {
        //     std::cout << "警告: users 表为空" << std::endl;
        //     std::cout << "[]" << std::endl;  // 输出空JSON数组
        // } else {
        //     // 输出 JSON 数组
        //     std::cout << "查询结果 JSON 数组:\n" << j_array.dump(4) << std::endl;
        // }
        
        // 调用函数2：参数化查询特定用户（示例查询ID为1的用户）
        std::cout << "=== 示例2：查询特定ID的用户 ===" << std::endl;
        int target_id = 1; // 示例用户ID
        json j_array = query_user_by_id(db_conn_str, target_id);
        if (j_array.empty()) {
            std::cout << "未找到ID为 " << target_id << " 的用户" << std::endl;
        } else {
            std::cout << "找到用户：" << std::endl;
            std::cout << j_array.dump(4) << "\n" << std::endl;
        }



    }
    catch (const std::exception &e)
    {
        std::cerr << "数据库操作错误: " << e.what() << "\n";
        return 3;
    }

    return 0;
}
