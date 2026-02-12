#include <soci/soci.h>
#include <soci/postgresql/soci-postgresql.h>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include "CConfig.h"

// 1. 根据您的表结构定义 C++ 对象（保持不变）
struct User {
    int id;
    std::string username;
    std::string full_name;
    std::string email;
    std::string phone;
    
    // 构造函数
    User(int id = 0, 
         std::string username = "", 
         std::string full_name = "", 
         std::string email = "", 
         std::string phone = "")
        : id(id), username(username), full_name(full_name), 
          email(email), phone(phone) {}

    // 添加输出方法
    void print() const 
    {
        std::cout << "ID: " << id << ", 姓名: " << username 
                  << ", 全名: " << full_name << ", 邮箱: " << email 
                  << ", 电话: " << phone << std::endl;
    }
    
};

// 2. 删除 type_conversion 特化，改用基础语法


// 查询单个用户的方法（使用多个 soci::into）
bool query_user_direct(soci::session& sql, int user_id, User& user) 
{
    // 分别定义变量接收每个字段
    int id = 0;
    std::string username;
    std::string full_name;
    std::string email;
    std::string phone;
    
    // 使用多个 soci::into 绑定变量
    sql << "SELECT id, username, full_name, email, phone FROM users WHERE id = :id",
        soci::into(id), soci::into(username), soci::into(full_name), 
        soci::into(email), soci::into(phone), soci::use(user_id);
    
    // 检查是否查询到数据（如果查询不到，变量保持原值）
    // 我们可以通过检查 soci::statement 的状态来确认
    user = User(id, username, full_name, email, phone);
    
    return (id != 0 || !username.empty()); // 简单判断是否有数据
}

int main() 
{
    try 
    {
        // 读取配置（保持不变）
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
        
        std::cout << "=== SOCI 基础语法演示 ===\n" << std::endl;
        // 演示 A：查询单个对象
        std::cout << "A. 查询单个用户 (ID=1):" << std::endl;
        {
            int target_id = 1;
            
            // 方法1：使用多个 soci::into（最简单可靠）
            int id;
            std::string username, full_name, email, phone;
            
            sql << "SELECT id, username, full_name, email, phone FROM users WHERE id = :id",
                soci::into(id), soci::into(username), soci::into(full_name),
                soci::into(email), soci::into(phone), soci::use(target_id);
            
            User user(id, username, full_name, email, phone);
            user.print();
        }

        // // 演示 B. 查看所有用户
        // std::cout << "\nB. 查询所有用户:" << std::endl;
        // {
        //     // 使用 soci::rowset<soci::row>
        //     soci::rowset<soci::row> rs = (sql.prepare << 
        //         "SELECT id, username, full_name, email, phone FROM users");
            
        //     for (const soci::row& r : rs) {
        //         User user;
        //         user.id = r.get<int>("id");
        //         user.username = r.get<std::string>("username");
        //         user.full_name = r.get<std::string>("full_name");
        //         user.email = r.get<std::string>("email");
        //         user.phone = r.get<std::string>("phone");
                
        //         user.print();
        //     }
        // }
        
        
    } catch (const std::exception& e) {
        std::cerr << "\n错误发生: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}