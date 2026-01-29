#include <soci/soci.h>
#include <soci/postgresql/soci-postgresql.h>
#include <pqxx/pqxx>
#include <iostream>
#include <vector>
#include <string>
#include "CConfig.h"

// 1. 根据您的表结构定义 C++ 对象
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
        std::cout << "ID: " << id << ", 姓名: " << username << ", 全名: " << full_name << ", 邮箱: " << email << ", 电话: " << phone << std::endl;
    }
};

// 2. SOCI 类型转换定义
namespace soci 
{
    template<>
    struct type_conversion<User> 
    {
        typedef values base_type;

        // 从数据库读取数据到 User 对象
        static void from_base(values const & v, indicator ind, User & s) {
            // 检查指示器（处理 NULL 值）
            if (ind == i_null) {
                s = User();  // 返回空对象
                return;
            }
            
            try 
            {
                // 注意：字段名必须和 SELECT 语句中的列名完全匹配
                // 如果数据库字段名包含空格，需要用引号，这里用对应的别名
                s.id = v.get<int>("id");
                s.username = v.get<std::string>("username");
                s.full_name = v.get<std::string>("full_name");
                s.email = v.get<std::string>("email");
                s.phone = v.get<std::string>("phone");
            } catch (const soci_error& e) 
            {
                std::cerr << "映射错误: " << e.what() << std::endl;
                std::cerr << "检查字段名是否匹配: id, username, full_name, email, phone" << std::endl;
                throw;
            }
        }

        // 从 User 对象写入数据库
        static void to_base(const User & s, values & v, indicator & ind) {
            try 
            {
                v.set("id", s.id);
                v.set("username", s.username);
                v.set("full_name", s.full_name);
                v.set("email", s.email);
                v.set("phone", s.phone);
                ind = i_ok;  // 所有字段都有值
            } catch (const soci_error& e) 
            {
                std::cerr << "设置错误: " << e.what() << std::endl;
                throw;
            }
        }
    };
}

int main() 
{
    try 
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
    
        soci::session sql(
            soci::postgresql, 
            db_conn_str
        );
        
        std::cout << "=== SOCI ORM 映射演示 ===\n" << std::endl;
        
        // 演示 A：查询单个对象
        std::cout << "A. 查询单个用户 (ID=1):" << std::endl;
        {
            User user;
            int target_id = 1;
            
            // 注意：列名必须和 from_base 中的 get() 参数一致
            sql << "SELECT id, username, full_name, email, phone FROM users WHERE id = :id",
                soci::into(user), soci::use(target_id);
            
            user.print();
        }
        
        
        // 演示 B. 查看所有用户
        std::cout << "\nB. 查询所有用户:" << std::endl;
        {
            std::vector<User> users;
            
            // 注意：列名必须和 from_base 中的 get() 参数一致
            soci::rowset<User> rs = (sql.prepare << "SELECT id, username, full_name, email, phone FROM users");
            
            for (const auto& u : rs) {
                users.push_back(u);
            }
            
            for (const auto& u : users) {
                u.print();
            }
        }
        
    } catch (const std::exception& e) {
        std::cerr << "\n错误发生: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}