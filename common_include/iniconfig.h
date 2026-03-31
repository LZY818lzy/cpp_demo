#pragma once

#include <string>
#include <map>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <cctype>

class CConfig
{

public:
    // 1、获取单例实例
    static CConfig &GetInstance()
    {
        static CConfig instance; // C++11 保证线程安全
        return instance;
    }

    // 2、加载配置文件
    bool Load(const std::string &confName)
    {
        std::ifstream file(confName);
        if (!file.is_open())
        {
            last_error_ = "Cannot open file: " + confName;
            return false;
        }

        config_.clear();
        std::string line;
        int line_num = 0;

        while (std::getline(file, line))
        {
            ++line_num;

            // 去除行首尾空白
            line = Trim(line);

            // 跳过空行和注释行
            if (line.empty() || line[0] == ';' || line[0] == '#')
            {
                continue;
            }

            // 查找等号分隔符
            std::string::size_type pos = line.find('=');
            if (pos == std::string::npos)
            {
                last_error_ = "Syntax error at line " + std::to_string(line_num) + ": " + line;
                return false;
            }

            std::string key = Trim(line.substr(0, pos));
            std::string value = Trim(line.substr(pos + 1));

            if (key.empty())
            {
                last_error_ = "Empty key at line " + std::to_string(line_num);
                return false;
            }

            // 去除value中行尾注释（未被引号包裹的部分）
            value = StripInlineComment(value);

            // 去除value两侧的引号
            value = StripQuotes(value);

            config_[key] = value;
        }

        return true;
    }

    // 3、获取最后的错误信息
    std::string GetLastError() const { return last_error_; }

    // 4、获取配置
    // 获取字符串
    std::string GetString(const std::string &itemname)
    {
        auto it = config_.find(itemname);
        if (it == config_.end())
        {
            throw std::runtime_error("Key not found: " + itemname);
        }
        return it->second;
    }

    std::string GetStringDefault(const std::string &itemname, const std::string &default_value)
    {
        auto it = config_.find(itemname);
        if (it != config_.end())
        {
            return it->second;
        }
        return default_value;
    }

    // 获取整型
    int GetInt(const std::string &itemname)
    {
        const std::string &val = GetString(itemname);
        try
        {
            return std::stoi(val);
        }
        catch (const std::exception &)
        {
            throw std::runtime_error("Cannot convert to int, key: " + itemname + ", value: " + val);
        }
    }

    int GetIntDefault(const std::string &itemname, int default_value)
    {
        auto it = config_.find(itemname);
        if (it != config_.end())
        {
            try
            {
                return std::stoi(it->second);
            }
            catch (const std::exception &)
            {
                return default_value;
            }
        }
        return default_value;
    }

    // 获取双精度浮点型
    double GetDouble(const std::string &itemname)
    {
        const std::string &val = GetString(itemname);
        try
        {
            return std::stod(val);
        }
        catch (const std::exception &)
        {
            throw std::runtime_error("Cannot convert to double, key: " + itemname + ", value: " + val);
        }
    }

    double GetDoubleDefault(const std::string &itemname, double default_value)
    {
        auto it = config_.find(itemname);
        if (it != config_.end())
        {
            try
            {
                return std::stod(it->second);
            }
            catch (const std::exception &)
            {
                return default_value;
            }
        }
        return default_value;
    }

    // 获取布尔型
    bool GetBool(const std::string &itemname)
    {
        const std::string &val = GetString(itemname);
        return ToBool(val, itemname);
    }

    bool GetBoolDefault(const std::string &itemname, bool default_value)
    {
        auto it = config_.find(itemname);
        if (it != config_.end())
        {
            std::string lower = ToLower(it->second);
            if (lower == "true" || lower == "1")
                return true;
            if (lower == "false" || lower == "0")
                return false;
            return default_value;
        }
        return default_value;
    }

    // 5、防止拷贝
    CConfig(const CConfig &) = delete;
    CConfig &operator=(const CConfig &) = delete;

private:
    CConfig() = default;
    ~CConfig() = default;

    std::map<std::string, std::string> config_;
    std::string last_error_;

    // 去除字符串首尾空白
    static std::string Trim(const std::string &s)
    {
        std::string::size_type start = s.find_first_not_of(" \t\r\n");
        if (start == std::string::npos)
            return "";
        std::string::size_type end = s.find_last_not_of(" \t\r\n");
        return s.substr(start, end - start + 1);
    }

    // 转小写
    static std::string ToLower(const std::string &s)
    {
        std::string result = s;
        std::transform(result.begin(), result.end(), result.begin(),
                       [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return result;
    }

    // 去除value两侧的引号（单引号或双引号）
    static std::string StripQuotes(const std::string &s)
    {
        if (s.size() >= 2)
        {
            char front = s.front();
            char back = s.back();
            if ((front == '"' && back == '"') || (front == '\'' && back == '\''))
            {
                return s.substr(1, s.size() - 2);
            }
        }
        return s;
    }

    // 去除行尾注释（不在引号内的 ; 或 #）
    static std::string StripInlineComment(const std::string &s)
    {
        if (s.empty())
            return s;

        char quote = '\0';
        for (std::string::size_type i = 0; i < s.size(); ++i)
        {
            char c = s[i];
            if (quote != '\0')
            {
                if (c == quote)
                    quote = '\0';
            }
            else
            {
                if (c == '"' || c == '\'')
                {
                    quote = c;
                }
                else if (c == ';' || c == '#')
                {
                    return Trim(s.substr(0, i));
                }
            }
        }
        return s;
    }

    // 字符串转布尔值
    static bool ToBool(const std::string &val, const std::string &key)
    {
        std::string lower = ToLower(val);
        if (lower == "true" || lower == "1")
            return true;
        if (lower == "false" || lower == "0")
            return false;
        throw std::runtime_error("Cannot convert to bool, key: " + key + ", value: " + val);
    }
};
