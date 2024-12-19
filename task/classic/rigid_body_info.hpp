#ifndef RIGID_BODY_INFO_HPP
#define RIGID_BODY_INFO_HPP

#include <btBulletDynamicsCommon.h>
#include <glm/glm.hpp>
#include <nlohmann/json.hpp>
#include <fstream>
#include <vector>
#include <string>

namespace GL_TASK
{

    // 物体类型枚举
    enum class ObjectType
    {
        NONE,
        BOAT,
        FLOWER
    };

    // 刚体信息类
    struct RigidBodyInfo
    {
        ObjectType objectType; // 物体类型（如船只、花朵等）
        std::string name;      // 物体名称
        btScalar mass;         // 物体质量
        btVector3 dimensions;  // 物体尺寸（例如：盒子的半长宽高）
        // btCollisionShape* collisionShape;  // 碰撞形状（例如：盒子、球体等）
        btScalar friction;     // 摩擦力系数
        btScalar restitution;  // 反弹系数
        glm::mat4 base_matrix; // 初始变换矩阵
    };

    // 刚体信息管理类
    class RigidBodyManager
    {
    public:
        // 构造函数
        RigidBodyManager(ObjectType type = ObjectType::NONE, const std::string &config_path = "rigid_body_config.json")
        {
            initConfig(config_path);
            chooseType(type);
        }

        void chooseType(ObjectType type)
        {
            switch (type)
            {
            case ObjectType::BOAT:
                info = getInfo("boat");
                break;
            case ObjectType::FLOWER:
                info = getInfo("flower");
                break;
            default:
                info = getInfo("default");
                break;
            }
            info.objectType = type;
        }

        // 返回刚体碰撞形状(新副本)
        btCollisionShape *getCollisionShape() const
        {
            switch (info.objectType)
            {
            case ObjectType::BOAT:
                return new btBoxShape(info.dimensions * 0.5f);
            case ObjectType::FLOWER:
                return new btCylinderShape(info.dimensions * 0.5f);
            default:
                return new btSphereShape(info.dimensions.length() * 0.5f);
            }
            return nullptr;
        }

        // 返回模型名称
        std::string getTypeName() const { return info.name; }

        // 返回刚体质量
        btScalar getMass() const { return info.mass; }

        // 返回摩擦系数
        btScalar getFriction() const { return info.friction; }

        // 返回反弹系数
        btScalar getRestitution() const { return info.restitution; }

        // 返回基础矩阵
        glm::mat4 getBaseMatrix() const { return info.base_matrix; }

    private:
        // 物体的基本信息
        RigidBodyInfo info;
        std::unordered_map<std::string, RigidBodyInfo> configs;

        // 初始化配置文件
        bool initConfig(const std::string &config_path)
        {
            std::ifstream file(config_path);
            if (!file.is_open())
            {
                std::cerr << "RigidBodyManager:: Failed to open configuration file: " << config_path << std::endl;
                return false;
            }

            try
            {
                nlohmann::json jsonConfig;
                file >> jsonConfig;

                for (const auto &[key, value] : jsonConfig.items())
                {
                    configs[key] = parseConfig(value);
                }
            }
            catch (const std::exception &e)
            {
                std::cerr << "Error parsing JSON: " << e.what() << std::endl;
                return false;
            }

            return true;
        }

        // 获取特定刚体信息
        const RigidBodyInfo &getInfo(const std::string &name) const
        {
            auto it = configs.find(name);
            if (it != configs.end())
            {
                return it->second;
            }
            static RigidBodyInfo defaultConfig;
            throw std::runtime_error("RigidBodyManager:: Configuration for '" + name + "' not found.");
        }

        // 从配置文件读取刚体信息
        RigidBodyInfo parseConfig(const nlohmann::json &config) const
        {
            // 解析配置文件
            RigidBodyInfo info;
            info.name = config.value("name", "default");
            info.mass = config.value("mass", 10.0f);
            double scale = config.value("scale", 1.0);
            info.dimensions = btVector3(
                config["dimensions"][0].get<float>() * float(scale),
                config["dimensions"][1].get<float>() * float(scale),
                config["dimensions"][2].get<float>() * float(scale));
            info.friction = config.value("friction", 0.0f);
            info.restitution = config.value("restitution", 0.0f);
            ;

            // 配置初始变换矩阵
            glm::vec3 offset = glm::vec3(
                config["offset"][0].get<double>(),
                config["offset"][1].get<double>(),
                config["offset"][2].get<double>());
            glm::mat4 base_matrix = glm::translate(glm::mat4(1.0f), offset);
            info.base_matrix = glm::scale(base_matrix, glm::vec3(scale));

            return info;
        }
    };
}

#endif // RIGID_BODY_INFO_HPP
