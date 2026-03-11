#include <httplib.h>
#include <nlohmann/json.hpp>
#include <iostream>
#include <csignal>
#include <unordered_map>

using json = nlohmann::json;

// 全局服务器指针，用于信号处理
httplib::Server* g_svr = nullptr;

void signal_handler(int signal) {
    std::cout << "\nReceived signal " << signal << ", stopping server..." << std::endl;
    if (g_svr) {
        g_svr->stop();
    }
}

int main() {
    // 创建服务器对象
    httplib::Server svr;
    g_svr = &svr;

    // 注册信号处理（Ctrl+C）
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    // 模拟数据源：按 id 查用户
    std::unordered_map<int, json> user_db = {
        {5, {{"id", 5}, {"name", "liu zhaoyuan"}, {"username", "lzy"}, {"email", "Lucio_Hettinger@annie.ca"},
             {"address", {{"street", "Skiles Walks"}, {"city", "Roscoeview"}}}}},
        {6, {{"id", 6}, {"name", "zhang san"}, {"username", "zs"}, {"email", "zhangsan@example.com"}}},
        {7, {{"id", 7}, {"name", "li si"}, {"username", "ls"}, {"email", "lisi@example.com"}}}
    };

    // 绑定资源 /users，通过参数 id 查询：/users?id=5
    svr.Get("/users", [&user_db](const httplib::Request& req, httplib::Response& res) {
        if (!req.has_param("id")) {
            res.status = 400;
            res.set_content(R"({"error":"missing query param: id"})", "application/json");
            return;
        }

        try {
            int id = std::stoi(req.get_param_value("id"));
            auto it = user_db.find(id);
            if (it == user_db.end()) {
                res.status = 404;
                res.set_content(R"({"error":"user not found"})", "application/json");
                return;
            }

            res.set_content(it->second.dump(), "application/json");
        } catch (...) {
            res.status = 400;
            res.set_content(R"({"error":"id must be integer"})", "application/json");
        }
    });

    std::cout << "Server started at http://127.0.0.1:8080\nPress Ctrl+C to stop." << std::endl;
    
    // listen 阻塞，直到 svr.stop() 被调用
    svr.listen("0.0.0.0", 8080);

    std::cout << "Server stopped." << std::endl;
    return 0;
}