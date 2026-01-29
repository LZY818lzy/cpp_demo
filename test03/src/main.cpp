#include <iostream>
#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// 定义结构体
namespace ns
{
    struct person
    {
        int id;
        std::string name;
        std::string fullname;
        std::string email;
        std::string phone;
    };
}

int main()
{
    std::cout << "=== nlohmann/json 自定义类型转换测试 ===\n\n";

    // 测试1: 基础序列化和反序列化
    std::cout << "测试1: 基础对象转换\n";
    {
        ns::person u1 = {1, "alice","张丽", "alice@example.com", "13800138001"};

        // 手动序列化
        json j;
        j["id"] = u1.id;
        j["name"] = u1.name;
        j["fullname"] = u1.fullname;
        j["email"] = u1.email;
        j["phone"] = u1.phone;

        std::cout << "手动序列化结果:\n";
        std::cout << j.dump(4) << "\n\n";

        // 手动反序列化
        ns::person u2 {
            j["id"].get<int>(),
            j["name"].get<std::string>(),
            j["fullname"].get<std::string>(),
            j["email"].get<std::string>(),
            j["phone"].get<std::string>()
        };
        
        std::cout << "手动反序列化验证:\n";
        std::cout << "ID: " << u2.id << "\n";
        std::cout << "用户名: " << u2.name << "\n";
        std::cout << "全名: " << u2.fullname << "\n";
        std::cout << "邮箱: " << u2.email << "\n";
        std::cout << "电话: " << u2.phone << "\n\n";
    }

    return 0;
}