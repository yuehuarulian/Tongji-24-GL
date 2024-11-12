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
            glm::vec3(107.25, 33.9, -82.75),     // bulb.001
            glm::vec3(107.25, 36.63, -10.666),   // bulb.002
            glm::vec3(107.31, 33.845, 61.95),    // bulb.003
            glm::vec3(107.18, 34.346, 136.48),   // bulb.004
            glm::vec3(107.25, -33.755, -84.019), // bulb.008
            glm::vec3(107.32, -34.331, -14.349), // bulb.009
            glm::vec3(106.93, -37.012, 60.997),  // bulb.010
            glm::vec3(106.89, -34.19, 135.34),   // bulb.011
        };
        std::vector<glm::vec3> area_lights_normal = {
            glm::vec3(0., 0., 1.),
            glm::vec3(-1., -0., -0.),
            glm::vec3(-1., -0., -0.),
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
