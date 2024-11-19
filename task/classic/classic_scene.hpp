#ifndef CLASSIC_SCENE_HPP
#define CLASSIC_SCENE_HPP

#include "scene.hpp"
#include "shader_manager.hpp"
#include "light_manager.hpp"
#include "renderable_model.hpp"
#include "point_cloud.hpp"
#include <btBulletDynamicsCommon.h>
#include <memory>
#include "boat.hpp"

namespace GL_TASK
{
    class ClassicScene : public Scene
    {
    public:
        ClassicScene(ShaderManager &shader_manager, LightManager &light_manager, float precision = 0.2);
        ~ClassicScene();

        void render(const glm::mat4 &projection, const glm::mat4 &view, glm::vec3 &camera_pos) override;

    private:
        std::vector<std::shared_ptr<PointCloud>> point_clouds;
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
            glm::vec3(0., -1., 0.),
            glm::vec3(0., -1., 0.),
            glm::vec3(0., -1., 0.),
            glm::vec3(0., -1., 0.),
            glm::vec3(0., -1., 0.),
            glm::vec3(0., -1., 0.),
            glm::vec3(0., -1., 0.),
            glm::vec3(0., -1., 0.),
        };

        float precision;
        float water_level = 0.0f;     // 水面高度
        float buoyancy_force = 10.0f; // 浮力强度
        float linear_damping = 0.2f;  // 线性阻尼
        float angular_damping = 0.3f; // 角速度阻尼

        btDiscreteDynamicsWorld *dynamicsWorld;
        std::shared_ptr<Boat> boatInstance;

        // 新增可调参数
        float waterDensity = 1000.0f;
        float dragCoefficient = 0.5f;

        // 允许动态调整水面高度
        float waterLevel = 0.0f;

        void setup_scene() override;
    };
}
#endif
