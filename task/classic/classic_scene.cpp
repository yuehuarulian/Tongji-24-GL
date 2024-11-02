
#include "classic_scene.hpp"

namespace GL_TASK
{
    ClassicScene::ClassicScene(ShaderManager &shader_manager, LightManager &light_manager) : Scene(shader_manager, light_manager)
    {
        setup_scene();
    }

    void ClassicScene::setup_scene()
    {

        // 加载着色器
        shader_manager.load_shader("roomShader", "source/shader/room.vert", "source/shader/room.frag");
        // shaderManager.loadShader("basicShader", "assets/shaders/basic.vert", "assets/shaders/basic.frag");

        // 创建模型并为每个模型分配着色器
        // room
        auto shader = shader_manager.get_shader("roomShader");
        light_manager.apply_lights(shader);
        auto room_model = std::make_shared<Room>("source/model/room.obj", shader, true);
        models.push_back(room_model);
    }

    void ClassicScene::render(const glm::mat4 &projection, const glm::mat4 &view, glm::vec3 &camera_pos)
    {
        for (const auto &model : models)
        {
            model->draw(projection, view, camera_pos);
        }
    }
}