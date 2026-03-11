#include <iostream>
#include <string>
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>

// 为了方便，通常会给 json 库起个别名
using json = nlohmann::json;

int main() {
    std::cout << "输入用户 id 发起查询（输入 q 退出）" << std::endl;

    std::string input;
    while (true) {
        std::cout << "\n请输入 id: ";
        if (!std::getline(std::cin, input)) {
            break;
        }

        if (input == "q" || input == "Q") {
            break;
        }

        int id = 0;
        try {
            id = std::stoi(input);
        } catch (...) {
            std::cerr << "id 必须是整数，请重新输入。" << std::endl;
            continue;
        }

        cpr::Response r = cpr::Get(
            cpr::Url{"http://127.0.0.1:8080/users"},
            cpr::Parameters{{"id", std::to_string(id)}}
        );

        std::cout << "状态码: " << r.status_code << std::endl;

        if (r.status_code == 200) {
            try {
                json userData = json::parse(r.text);
                std::cout << "应答: " << userData.dump(4) << std::endl;
            } catch (const json::parse_error&) {
                std::cout << "应答: " << r.text << std::endl;
            }
        } else {
            std::cerr << "请求失败，服务端返回: " << r.text << std::endl;
            if (!r.error.message.empty()) {
                std::cerr << "网络错误: " << r.error.message << std::endl;
            }
        }
    }

    return 0;
}