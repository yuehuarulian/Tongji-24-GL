// LightManager.hpp
#ifndef LIGHT_MANAGER_HPP
#define LIGHT_MANAGER_HPP

#include <vector>
#include <memory>
#include "Shader.hpp"
#include <glm/glm.hpp>
// 光源结构体定义

struct PointLight // 点光源
{
    glm::vec3 position; // 位置
    float constant;     // 常数项
    float linear;       // 一次项
    float quadratic;    // 二次项
    glm::vec3 ambient;  // 环境光
    glm::vec3 diffuse;  // 漫反射
    glm::vec3 specular; // 镜面反射
};

struct DirectionalLight // 方向光
{
    glm::vec3 direction; // 方向
    glm::vec3 ambient;   // 环境光
    glm::vec3 diffuse;   // 漫反射
    glm::vec3 specular;  // 镜面反射
};

struct SpotLight // 聚光灯
{
    glm::vec3 position;  // 位置
    glm::vec3 direction; // 方向
    float cutOff;        // 内切光角
    float outerCutOff;   // 外切光角
    float constant;      // 常数项
    float linear;        // 一次项
    float quadratic;     // 二次项
    glm::vec3 ambient;   // 环境光
    glm::vec3 diffuse;   // 漫反射
    glm::vec3 specular;  // 镜面反射
};

class LightManager
{
public:
    void add_point_light(const glm::vec3 &position, const glm::vec3 &color, float intensity);
    void add_directional_light(const glm::vec3 &position, const glm::vec3 &color, float intensity);
    void add_spot_light(const glm::vec3 &position, const glm::vec3 &color, float intensity);
    void apply_lights(const std::shared_ptr<Shader> &shader);

private:
    std::vector<PointLight> point_lights;
    std::vector<DirectionalLight> directional_lights;
    std::vector<SpotLight> spot_lights;
};

#endif
