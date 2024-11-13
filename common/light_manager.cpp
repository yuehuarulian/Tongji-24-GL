// LightManager.cpp
#include "light_manager.hpp"

void LightManager::add_point_light(const glm::vec3 &position, const glm::vec3 &color, float constant, float linear, float quadratic)
{
    PointLight light(position, color, constant, linear, quadratic);
    point_lights.push_back(light);
}

void LightManager::add_directional_light(const glm::vec3 &direction, const glm::vec3 &color)
{
    DirectionalLight light(direction, color);
    directional_lights.push_back(light);
}

void LightManager::add_spot_light(const glm::vec3 &position, const glm::vec3 direction, const glm::vec3 &color, float cutOff, float outerCutOff, float constant, float linear, float quadratic)
{
    SpotLight light(position, direction, color, cutOff, outerCutOff, constant, linear, quadratic);
    spot_lights.push_back(light);
}

void LightManager::add_area_light(const glm::vec3 &position, const glm::vec3 &normal, const glm::vec3 &color, float width, float height, int num_samples)
{
    AreaLight light(position, normal, color, width, height, num_samples);
    area_lights.push_back(light);
}

void LightManager::clear_lights()
{
    point_lights.clear();
    directional_lights.clear();
    spot_lights.clear();
    area_lights.clear();
}

void LightManager::apply_lights(const std::shared_ptr<Shader> &shader)
{
    shader->use();

    // 传递 PointLights
    for (size_t i = 0; i < point_lights.size(); ++i)
    {
        shader->setVec3("point_lights[" + std::to_string(i) + "].position", point_lights[i].position);
        shader->setVec3("point_lights[" + std::to_string(i) + "].color", point_lights[i].color);
        shader->setFloat("point_lights[" + std::to_string(i) + "].constant", point_lights[i].constant);
        shader->setFloat("point_lights[" + std::to_string(i) + "].linear", point_lights[i].linear);
        shader->setFloat("point_lights[" + std::to_string(i) + "].quadratic", point_lights[i].quadratic);
    }
    if (point_lights.size() > 0)
        shader->setInt("num_point_lights", point_lights.size());

    // 传递 DirectionalLights
    for (size_t i = 0; i < directional_lights.size(); ++i)
    {
        shader->setVec3("directional_lights[" + std::to_string(i) + "].direction", directional_lights[i].direction);
        shader->setVec3("directional_lights[" + std::to_string(i) + "].color", directional_lights[i].color);
    }
    if (directional_lights.size() > 0)
        shader->setInt("num_directional_lights", directional_lights.size());

    // 传递 SpotLights
    for (size_t i = 0; i < spot_lights.size(); ++i)
    {
        shader->setVec3("spot_lights[" + std::to_string(i) + "].position", spot_lights[i].position);
        shader->setVec3("spot_lights[" + std::to_string(i) + "].direction", spot_lights[i].direction);
        shader->setVec3("spot_lights[" + std::to_string(i) + "].color", spot_lights[i].color);
        shader->setFloat("spot_lights[" + std::to_string(i) + "].cutOff", spot_lights[i].cutOff);
        shader->setFloat("spot_lights[" + std::to_string(i) + "].outerCutOff", spot_lights[i].outerCutOff);
        shader->setFloat("spot_lights[" + std::to_string(i) + "].constant", spot_lights[i].constant);
        shader->setFloat("spot_lights[" + std::to_string(i) + "].linear", spot_lights[i].linear);
        shader->setFloat("spot_lights[" + std::to_string(i) + "].quadratic", spot_lights[i].quadratic);
    }
    if (spot_lights.size() > 0)
        shader->setInt("num_spot_lights", spot_lights.size());

    // 传递 AreaLights
    for (size_t i = 0; i < area_lights.size(); i++)
    {
        const auto &light = area_lights[i];
        shader->setVec3("area_lights[" + std::to_string(i) + "].position", light.position);
        shader->setVec3("area_lights[" + std::to_string(i) + "].normal", light.normal);
        shader->setVec3("area_lights[" + std::to_string(i) + "].color", light.color / static_cast<float>(light.num_samples));
        shader->setFloat("area_lights[" + std::to_string(i) + "].width", light.width);
        shader->setFloat("area_lights[" + std::to_string(i) + "].height", light.height);
        shader->setInt("area_lights[" + std::to_string(i) + "].num_samples", light.num_samples);
    }
    if (area_lights.size() > 0)
        shader->setInt("num_area_lights", area_lights.size());
}
