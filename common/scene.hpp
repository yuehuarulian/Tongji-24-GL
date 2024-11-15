#ifndef SCENE_HPP
#define SCENE_HPP
#include <vector>
#include <memory>
#include "room.hpp"
#include "shader_manager.hpp"
#include "renderable_model.hpp"
#include "light_manager.hpp"

class Scene
{
public:
    Scene(ShaderManager &shader_manager, LightManager &light_manager) : shader_manager(shader_manager), light_manager(light_manager) {}

    virtual ~Scene() = default;

    virtual void render(const glm::mat4 &projection, const glm::mat4 &view, glm::vec3 &camera_pos) = 0;

protected:
    virtual void setup_scene() = 0;

    ShaderManager &shader_manager;
    LightManager &light_manager;
};

#endif // SCENE_HPP