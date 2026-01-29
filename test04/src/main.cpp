#include <iostream>
#include <string>
#include <vector>
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

    void to_json(json &j, const person &p)
    {
        j = json{
            {"id", p.id},
            {"name", p.name},
            {"fullname", p.fullname},
            {"email", p.email},
            {"phone", p.phone}};
    }

    void from_json(const json &j, person &p)
    {
        j.at("id").get_to(p.id);
        j.at("name").get_to(p.name);
        j.at("fullname").get_to(p.fullname);
        j.at("email").get_to(p.email);
        j.at("phone").get_to(p.phone);
    }
}

int main()
{
    std::cout << "=== 测试person结构体的ADL序列化 ===\n\n";

    // 测试1：基本ADL序列化
    ns::person p1 = {1, "alice", "张丽", "alice@example.com", "13800138001"};

    json j = p1; // 调用to_json

    std::cout << "序列化结果:\n";
    std::cout << j.dump(4) << "\n\n";

    // 测试2：基本ADL反序列化
    ns::person p2 = j.get<ns::person>();// 调用from_json

    std::cout << "反序列化结果:\n";
    std::cout << "id: " << p2.id << "\n";
    std::cout << "name: " << p2.name << "\n";
    std::cout << "fullname: " << p2.fullname << "\n";
    std::cout << "email: " << p2.email << "\n";
    std::cout << "phone: " << p2.phone << "\n";

    return 0;
}