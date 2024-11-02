// LightManager.cpp
#include "light_manager.hpp"

void LightManager::add_point_light(const glm::vec3 &position, const glm::vec3 &color, float intensity)
{
    // point_lights.push_back();
}

void LightManager::add_directional_light(const glm::vec3 &position, const glm::vec3 &color, float intensity)
{
    // directional_lights.push_back();
}

void LightManager::add_spot_light(const glm::vec3 &position, const glm::vec3 &color, float intensity)
{
    // spot_lights.push_back();
}

void LightManager::apply_lights(const std::shared_ptr<Shader> &shader)
{

    // 传递 PointLights
    for (size_t i = 0; i < point_lights.size(); ++i)
    {
        shader->setVec3("point_lights[" + std::to_string(i) + "].position", point_lights[i].position);
        shader->setFloat("point_lights[" + std::to_string(i) + "].constant", point_lights[i].constant);
        shader->setFloat("point_lights[" + std::to_string(i) + "].linear", point_lights[i].linear);
        shader->setFloat("point_lights[" + std::to_string(i) + "].quadratic", point_lights[i].quadratic);
        shader->setVec3("point_lights[" + std::to_string(i) + "].ambient", point_lights[i].ambient);
        shader->setVec3("point_lights[" + std::to_string(i) + "].diffuse", point_lights[i].diffuse);
        shader->setVec3("point_lights[" + std::to_string(i) + "].specular", point_lights[i].specular);
    }

    // 传递 DirectionalLights
    for (size_t i = 0; i < directional_lights.size(); ++i)
    {
        shader->setVec3("directional_lights[" + std::to_string(i) + "].direction", directional_lights[i].direction);
        shader->setVec3("directional_lights[" + std::to_string(i) + "].ambient", directional_lights[i].ambient);
        shader->setVec3("directional_lights[" + std::to_string(i) + "].diffuse", directional_lights[i].diffuse);
        shader->setVec3("directional_lights[" + std::to_string(i) + "].specular", directional_lights[i].specular);
    }

    // 传递 SpotLights
    for (size_t i = 0; i < spot_lights.size(); ++i)
    {
        shader->setVec3("spot_lights[" + std::to_string(i) + "].position", spot_lights[i].position);
        shader->setVec3("spot_lights[" + std::to_string(i) + "].direction", spot_lights[i].direction);
        shader->setFloat("spot_lights[" + std::to_string(i) + "].cutOff", spot_lights[i].cutOff);
        shader->setFloat("spot_lights[" + std::to_string(i) + "].outerCutOff", spot_lights[i].outerCutOff);
        shader->setFloat("spot_lights[" + std::to_string(i) + "].constant", spot_lights[i].constant);
        shader->setFloat("spot_lights[" + std::to_string(i) + "].linear", spot_lights[i].linear);
        shader->setFloat("spot_lights[" + std::to_string(i) + "].quadratic", spot_lights[i].quadratic);
        shader->setVec3("spot_lights[" + std::to_string(i) + "].ambient", spot_lights[i].ambient);
        shader->setVec3("spot_lights[" + std::to_string(i) + "].diffuse", spot_lights[i].diffuse);
        shader->setVec3("spot_lights[" + std::to_string(i) + "].specular", spot_lights[i].specular);
    }
}
