#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <Basic/File.hpp>
#include <Basic/LStorage.hpp>

namespace Basic
{
    // 用于加载、解析和存储配置文件中的数据
    class Config
    {
    public:
        Config();
        Config(const std::string &fileName);    // 从该文件加载配置
        bool Load(const std::string &fileName); // 用于加载配置文件

        const std::string *GetStrPtr(const std::string &id) const;          // 获取字符串类型的配置
        const float *GetFloatPtr(const std::string &id) const;              // 获取浮动数值类型的配置
        const int *GetIntPtr(const std::string &id) const;                  // 获取整数类型的配置
        const unsigned int *GetUnsignedIntPtr(const std::string &id) const; // 获取无符号整数类型的配置

        // 获取并返回配置值，提供默认值
        bool GetVal(const std::string &id, float &val, float defaultVal = 0) const;
        bool GetVal(const std::string &id, int &val, int defaultVal = 0) const;
        bool GetVal(const std::string &id, unsigned int &val, unsigned int defaultVal = 0) const;
        bool GetVal(const std::string &id, std::string &val, const std::string &defaultVal = "") const;

        bool IsValid() const;

    private:
        // 配置文件解析
        bool Config::DecodeLine(const std::string &data);

        // 数据存储
        LStorage<std::string> strDirectory;
        LStorage<float> floatDirectory;
        LStorage<int> intDirectory;
        LStorage<unsigned int> unsignedIntDirectory;

        // 配置是否有效
        bool valid;
    };
}

#endif