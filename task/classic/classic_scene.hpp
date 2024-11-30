#ifndef CLASSIC_SCENE_HPP
#define CLASSIC_SCENE_HPP

#include "scene.hpp"
#include "shader_manager.hpp"
#include "light_manager.hpp"
#include "renderable_model.hpp"
#include "point_cloud.hpp"
#include "camera_control.hpp"
#include "butterfly.hpp"
#include <memory>

namespace GL_TASK
{
    class ClassicScene : public Scene
    {
    public:
        ClassicScene(ShaderManager &shader_manager, LightManager &light_manager, const int WINDOW_WIDTH = 1080, const int WINDOW_HEIGHT = 720);
        ~ClassicScene() = default;

        void render(Camera &) override;
        void present() override; // 显示结果到屏幕
        void update() override;  // 更新场景状态
        void wait_until_next_frame(int frame_number) override;

    private:
        void setup_scene();                 // 场景初始化
        void load_shaders();                // 加载着色器
        void load_models();                 // 加载模型
        void load_lights();                 // 加载光源
        void render_path_tracing(Camera &); // 路径追踪阶段
        void render_accumulation();         // 累积阶段
        void render_post_processing();      // 后处理阶段
        void render_point_clouds(Camera &); // 绘制点云

        // buttefly
        const int butterfly_count = 10;
        std::vector<std::shared_ptr<Butterfly>> butterflies;
        std::vector<std::shared_ptr<PointCloud>> point_clouds;

        // 光源
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
    };
}

#endif
