#include <iostream>
#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace ns
{
    struct address
    {
        std::string city;
        std::string street;
        int zipcode;
    };

    struct person
    {
        int id;
        std::string name;
        std::string fullname;
        std::string email;
        std::string phone;
        address addr;
    };
}

// // 定义转换（使用宏简化）
// namespace nlohmann
// {
//     NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ns::address, city, street, zipcode)
//     NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ns::person, id, name, fullname, email, phone, addr)
// }

int main()
{
    // // 创建对象
    // ns::person p = {
    //     1, "alice", "张丽", "alice@example.com", "13800138001", {"北京", "朝阳路123号", 100020}};

    // // 序列化
    // json j = p;

    // std::cout << "完整 JSON:\n";
    // std::cout << j.dump(4) << "\n\n";

    // // 测试各种访问方式
    // std::cout << "=== 访问嵌套字段 ===\n";

    // // 方式1: 链式访问
    // std::cout << "j[\"addr\"][\"city\"] = " << j["addr"]["city"] << "\n";
    // std::cout << "j[\"addr\"][\"street\"] = " << j["addr"]["street"] << "\n";
    // std::cout << "j[\"addr\"][\"zipcode\"] = " << j["addr"]["zipcode"] << "\n\n";

    // // 修改嵌套字段
    // std::cout << "=== 修改嵌套字段 ===\n";
    // j["addr"]["city"] = "上海";
    // j["addr"]["street"] = "南京路88号";

    // std::cout << "修改后的 JSON:\n";
    // std::cout << j.dump(4) << "\n\n";

    // // 反序列化回对象
    // ns::person p2 = j.get<ns::person>();
    // std::cout << "反序列化验证 - 城市: " << p2.addr.city << "\n";
    // std::cout << "反序列化验证 - 街道: " << p2.addr.street << "\n";

    // // 直接访问嵌套字段（不使用宏定义的类型转换）
    // // 方法1: 完全手动展开嵌套
    // json j1;
    // j1["id"] = p.id;
    // j1["name"] = p.name;
    // j1["fullname"] = p.fullname;
    // j1["email"] = p.email;
    // j1["phone"] = p.phone;
    // j1["addr"]["city"] = p.addr.city;
    // j1["addr"]["street"] = p.addr.street;
    // j1["addr"]["zipcode"] = p.addr.zipcode;

    // std::cout << j1.dump(4) << "\n\n";

    // // 方法2: 先创建地址的JSON对象
    // json addrJson;
    // addrJson["city"] = p.addr.city;
    // addrJson["street"] = p.addr.street;
    // addrJson["zipcode"] = p.addr.zipcode;

    // // 再创建person的JSON对象
    // json j2;
    // j2["id"] = p.id;
    // j2["name"] = p.name;
    // j2["fullname"] = p.fullname;
    // j2["email"] = p.email;
    // j2["phone"] = p.phone;
    // j2["addr"] = addrJson; // 直接赋值子对象

    // std::cout << j2.dump(4) << "\n\n";

    // 反序列化时直接访问嵌套字段
    // 假设从某处获取的JSON数据
    json j = {
        {"id", 1},
        {"name", "alice"},
        {"fullname", "张丽"},
        {"email", "alice@example.com"},
        {"phone", "13800138001"},
        {"addr", {{"city", "北京"}, {"street", "朝阳路123号"}, {"zipcode", 100020}}}};
    // 方法1: 直接链式访问读取
    ns::person p;
    p.id = j["id"];
    p.name = j["name"];
    p.fullname = j["fullname"];
    p.email = j["email"];
    p.phone = j["phone"];

    // 嵌套访问：直接使用 ["key1"]["key2"]
    p.addr.city = j["addr"]["city"];
    p.addr.street = j["addr"]["street"];
    p.addr.zipcode = j["addr"]["zipcode"];

    std::cout << "ID: " << p.id << "\n";
    std::cout << "城市: " << p.addr.city << "\n";
    std::cout << "街道: " << p.addr.street << "\n\n";

    return 0;
}