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
    glm::vec3 color;    // 颜色
    float constant;     // 常数项
    float linear;       // 一次项
    float quadratic;    // 二次项
    // glm::vec3 ambient;  // 环境光
    // glm::vec3 diffuse;  // 漫反射
    // glm::vec3 specular; // 镜面反射

    PointLight(glm::vec3 position, glm::vec3 color, float constant, float linear, float quadratic)
        : position(position), color(color), constant(constant), linear(linear), quadratic(quadratic) {}
};

struct DirectionalLight // 方向光
{
    glm::vec3 direction; // 方向
    glm::vec3 color;     // 颜色
    // glm::vec3 ambient;   // 环境光
    // glm::vec3 diffuse;   // 漫反射
    // glm::vec3 specular;  // 镜面反射

    DirectionalLight(glm::vec3 direction, glm::vec3 color)
        : direction(direction), color(color) {}
};

struct SpotLight // 聚光灯
{
    glm::vec3 position;  // 位置
    glm::vec3 direction; // 方向
    glm::vec3 color;     // 颜色
    float cutOff;        // 内切光角
    float outerCutOff;   // 外切光角
    float constant;      // 常数项
    float linear;        // 一次项
    float quadratic;     // 二次项
    // glm::vec3 ambient;   // 环境光
    // glm::vec3 diffuse;   // 漫反射
    // glm::vec3 specular;  // 镜面反射

    SpotLight(glm::vec3 position, glm::vec3 direction, glm::vec3 color, float cutOff, float outerCutOf, float constant, float linear, float quadratic)
        : position(position), direction(direction), color(color), cutOff(cutOff), outerCutOff(outerCutOff), constant(constant), linear(linear), quadratic(quadratic) {}
};

struct AreaLight // 面光源
{
    glm::vec3 position; // 光源中心位置
    glm::vec3 normal;   // 光源朝向
    glm::vec3 color;    // 光源颜色
    float width;        // 光源的宽度
    float height;       // 光源的高度
    int num_samples;    // 采样点数量

    AreaLight(glm::vec3 position, glm::vec3 normal, glm::vec3 color, float width, float height, int num_samples)
        : position(position), normal(normal), color(color), width(width), height(height), num_samples(num_samples) {}
};

class LightManager
{
public:
    void add_point_light(const glm::vec3 &position, const glm::vec3 &color, float constant = 1.0f, float linear = 0.09f, float quadratic = 0.032f);
    void add_directional_light(const glm::vec3 &direction, const glm::vec3 &color);
    void add_spot_light(const glm::vec3 &position, const glm::vec3 direction, const glm::vec3 &color, float cutOff = glm::cos(glm::radians(12.5f)), float outerCutOff = glm::cos(glm::radians(17.5f)), float constant = 1.0f, float linear = 0.09f, float quadratic = 0.032f);
    void add_area_light(const glm::vec3 &position, const glm::vec3 &normal, const glm::vec3 &color, float width = 1.0f, float height = 1.0f, int num_samples = 16);

    void apply_lights(const std::shared_ptr<Shader> &shader);

    void clear_lights();

private:
    std::vector<PointLight> point_lights;
    std::vector<DirectionalLight> directional_lights;
    std::vector<SpotLight> spot_lights;
    std::vector<AreaLight> area_lights;

    // 辅助函数：生成网面光源采样点
    // std::vector<glm::vec3> generate_area_light_samples(const AreaLight &light);
};

#endif
