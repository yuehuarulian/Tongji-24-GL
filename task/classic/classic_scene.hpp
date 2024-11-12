#ifndef CLASSIC_SCENE_HPP
#define CLASSIC_SCENE_HPP

#include "scene.hpp"
#include "shader_manager.hpp"
#include "light_manager.hpp"
#include "renderable_model.hpp"
#include <memory>

namespace GL_TASK
{
    class ClassicScene : public Scene
    {
    public:
        ClassicScene(ShaderManager &shader_manager, LightManager &light_manager);
        ~ClassicScene() = default;

        void render(const glm::mat4 &projection, const glm::mat4 &view, glm::vec3 &camera_pos) override;

    private:
        std::vector<glm::vec3> area_lights_position = {
            glm::vec3(33.90, 107.25, -82.75),   // bulb.001  xyz
            glm::vec3(36.63, 107.25, -10.66),   // bulb.002
            glm::vec3(33.85, 107.31, 61.95),    // bulb.003
            glm::vec3(34.35, 107.18, 136.48),   // bulb.004
            glm::vec3(-33.76, 107.25, -84.019), // bulb.008
            glm::vec3(-34.33, 107.32, -14.349), // bulb.009
            glm::vec3(-37.01, 106.93, 60.997),  // bulb.010
            glm::vec3(-34.19, 106.89, 135.34),  // bulb.011
        };
        std::vector<glm::vec3> area_lights_normal = {
            glm::vec3(0., 0., 1.),
            glm::vec3(0., 0., 1.),
            glm::vec3(0., 0., 1.),
            glm::vec3(-1., -0., -0.),
            glm::vec3(-1., -0., -0.),
            glm::vec3(-1., -0., -0.),
            glm::vec3(-1., -0., -0.),
            glm::vec3(-1., -0., -0.),
        };
        glm::mat4 room_model_matrix = glm::mat4(1.0f);

        void setup_scene() override;
    };
}
#endif
