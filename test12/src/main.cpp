#include <iostream>
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>

// 为了方便，通常会给 json 库起个别名
using json = nlohmann::json;

int main() {
    // 1. 设置目标资源地址
    std::string url = "https://jsonplaceholder.typicode.com/users/5";

    // 2. 发送 GET 请求
    cpr::Response r = cpr::Get(cpr::Url{url});

    // 3. 检查响应状态
    if (r.status_code == 200) {
        // 4. 将字符串格式的响应体解析为 JSON 对象
        try {
            json userData = json::parse(r.text);

            // 5. 访问 JSON 中的特定字段
            std::string name = userData["name"];
            std::string email = userData["email"];
            std::string city = userData["address"]["city"];

            std::cout << "--- 用户信息 ---" << std::endl;
            std::cout << "姓名: " << name << std::endl;
            std::cout << "邮箱: " << email << std::endl;
            std::cout << "城市: " << city << std::endl;
            
            // 也可以直接打印整齐的 JSON 字符串 (缩进4格)
            // std::cout << userData.dump(4) << std::endl;

        } catch (json::parse_error& e) {
            std::cerr << "JSON 解析错误: " << e.what() << std::endl;
        }
    } else {
        std::cerr << "请求失败！状态码: " << r.status_code << std::endl;
        std::cerr << "错误信息: " << r.error.message << std::endl;
    }

    return 0;
}