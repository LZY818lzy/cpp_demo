// 目标实现：
// 通过配置文件设置日志输出，可以选择后台文件输出和控制台输出
// - 默认：后台日志文件输出
// - 配置文件里设置开关打开：控制台和后台日志文件一起输出

#include <iostream>
#include <filesystem>
#include <vector>
#include <thread>
#include <unistd.h>   // fork, getpid
#include <signal.h>   // signal handling
#include <fcntl.h>    // open, O_RDWR
#include <sys/stat.h> // umask

// spdlog 相关头文件
#include "spdlog/spdlog.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"

// 配置类头文件
#include "CConfig.h"

bool bExit = false; //  程序退出

// 全局日志器
std::shared_ptr<spdlog::logger> g_logger;

// 信号处理函数
void signalHandler(int signum)
{
    if (signum == SIGINT)
    {
        g_logger->warn("收到 SIGINT 信号 (Ctrl+C)，准备退出...");
    }
    else if (signum == SIGTERM)
    {
        g_logger->warn("收到 SIGTERM 信号 (kill命令)，准备退出...");
    }

    bExit = true;
}

/*
 * 将字符串级别转换为 spdlog 内部枚举
 */
spdlog::level::level_enum stringToLevel(const std::string &level_str)
{
    if (level_str == "trace")
        return spdlog::level::trace;
    if (level_str == "debug")
        return spdlog::level::debug;
    if (level_str == "info")
        return spdlog::level::info;
    if (level_str == "warn")
        return spdlog::level::warn;
    if (level_str == "error")
        return spdlog::level::err;
    if (level_str == "critical")
        return spdlog::level::critical;
    if (level_str == "off")
        return spdlog::level::off;
    return spdlog::level::info; // 默认级别
}
// --- 核心：真守护进程转换函数(标准流程) ---
void becomeDaemon(bool keep_console = false)
{
    // 1. 保存原始工作目录（防止切换到 / 后找不到配置文件和日志路径）
    std::string original_cwd = std::filesystem::current_path().string();

    g_logger->info("开始转换为守护进程...");

    // 2. 第一次 Fork
    pid_t pid = fork();
    if (pid < 0)
        exit(EXIT_FAILURE);
    if (pid > 0)
        exit(EXIT_SUCCESS); // 父进程退出

    // 3. 创建新会话，脱离终端
    if (setsid() < 0)
        exit(EXIT_FAILURE);

    // 4. 第二次 Fork（可选，防止进程重新取得终端控制权）
    pid = fork();
    if (pid < 0)
        exit(EXIT_FAILURE);
    if (pid > 0)
        exit(EXIT_SUCCESS);

    // 5. 设置文件权限掩码
    umask(0);

    // 6. 切换回原目录（或者根据需要切换到 /）
    chdir(original_cwd.c_str());

    // 7. 关闭并重定向标准流
    // ⚠️⚠️⚠️ 关键修改：根据配置决定是否重定向标准流
    if (!keep_console)
    {
        // 关闭并重定向标准流到黑洞
        int null_fd = open("/dev/null", O_RDWR);
        if (null_fd != -1)
        {
            dup2(null_fd, STDIN_FILENO);
            dup2(null_fd, STDOUT_FILENO);
            dup2(null_fd, STDERR_FILENO);
            close(null_fd);
        }
    }

    g_logger->info("已转换为守护进程运行（PID: {}）", getpid());

    // 重新注册信号处理器
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
}

/**
 * 初始化日志系统
 */
bool initLogging(CConfig &config)
{
    try
    {
        // 1. 获取日志配置参数
        bool log_console = config.GetBoolDefault("log_console", false); // 默认关闭控制台
        std::string level_str = config.GetStringDefault("level", "info");
        std::string pattern = config.GetStringDefault("pattern",
                                                      "[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
        std::string filename = config.GetStringDefault("filename", "logs/myservice.log");
        bool immediate_flush = config.GetBoolDefault("immediate_flush", true);
        int max_size = config.GetIntDefault("max_size", 10);
        int max_files = config.GetIntDefault("max_files", 3);

        // 2. 预处理：创建日志目录
        std::filesystem::path file_path(filename);

        if (file_path.has_parent_path())
        {
            std::filesystem::create_directories(file_path.parent_path());
        }

        // 3. 构建 Sinks 列表
        std::vector<spdlog::sink_ptr> sinks;

        // 策略 A：如果开启了控制台开关，添加彩色输出 Sink
        if (log_console)
        {
            auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            console_sink->set_pattern(pattern);
            sinks.push_back(console_sink);
        }

        // 策略 B：始终添加后台滚动文件 Sink（守护进程的核心）
        auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            filename, max_size * 1024, max_files);
        file_sink->set_pattern(pattern);
        sinks.push_back(file_sink);

        // 5. 实例化并注册全局 Logger
        g_logger = std::make_shared<spdlog::logger>("daemon_logger",
                                                    sinks.begin(), sinks.end());

        // 设置日志级别
        g_logger->set_level(stringToLevel(level_str));

        // 设置立即刷新
        if (immediate_flush)
        {
            g_logger->flush_on(stringToLevel(level_str));
        }

        // 设置为全局默认日志器
        spdlog::set_default_logger(g_logger);

        return true;
    }
    catch (const std::exception &e)
    {
        std::cerr << "日志初始化异常: " << e.what() << std::endl;
        return false;
    }
    catch (...)
    {
        std::cerr << "未知异常" << std::endl;
        return false;
    }
}

int main()
{
    // 1. 加载配置文件
    auto &config = CConfig::GetInstance();
    std::string configPath = "../config/spdlog.yaml"; // 使用绝对路径（相对于可执行文件）
    if (!config.Load(configPath))
    {
        std::cerr << "警告: 配置文件加载失败 (" << config.GetLastError()
                  << ")，将使用代码默认参数" << std::endl;
    }

    // 2. 初始化日志系统
    if (!initLogging(config))
    {
        return EXIT_FAILURE;
    }

    // 3. 根据配置决定是否进入“守护进程”模式
    bool daemonMode = config.GetBoolDefault("daemonMode", false);   // 默认前台运行
    bool log_console = config.GetBoolDefault("log_console", false); // 默认关闭控制台
    // 注册信号处理器
    signal(SIGINT, signalHandler);  // 处理 Ctrl+C
    signal(SIGTERM, signalHandler); // 处理 kill 命令
    g_logger->info("信号处理器已注册 (SIGINT, SIGTERM)");

    // 判断前台还是守护进程模式
    if (daemonMode)
    {
        g_logger->info("检测到守护进程配置，准备脱离终端...");
        becomeDaemon(log_console);
    }
    else
    {
        g_logger->info("未配置守护进程模式，继续在前台运行...");
    }

    // 4. 正式业务逻辑
    g_logger->info("进程初始化完成，当前 PID: {}, 工作目录: {}",
                   getpid(), std::filesystem::current_path().string());

    while (!bExit)
    {
        g_logger->info("服务运行中...");
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }

    // 确保所有日志写入文件
    g_logger->flush();
    g_logger->info("程序正常退出");

    // 清理日志系统
    spdlog::shutdown();

    return 0;
}