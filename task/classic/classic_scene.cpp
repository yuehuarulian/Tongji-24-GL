
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
        shader_manager.load_shader("room_shader", "source/shader/classic/room.vs", "source/shader/classic/room.fs");
        // shader_manager.load_shader("room_shader", "source/shader/pbr.vs", "source/shader/pbr_texture.fs");
        // shader_manager.load_shader("room_shader", "source/shader/classic/room.vert", "source/shader/classic/room.frag");

        // 创建模型并为每个模型分配着色器
        // room
        auto shader = shader_manager.get_shader("room_shader");
        light_manager.apply_lights(shader);
        // auto room_model = std::make_shared<Room>("source/model/room.obj", shader, true);
        // auto room_model = std::make_shared<Room>("source/model/room/overall.obj", shader, true);
        auto room_model = std::make_shared<Room>("C:/Users/13294/Desktop/ToyEffects/ToyEffects/assets/SceneModels/tree1/trees9.obj", shader, true);
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