
#include "classic_scene.hpp"
#include "draw_base_model.hpp"

namespace GL_TASK
{
    ClassicScene::ClassicScene(ShaderManager &shader_manager, LightManager &light_manager) : Scene(shader_manager, light_manager)
    {
        setup_scene();
    }

    // 对场景进行设置
    void ClassicScene::setup_scene()
    {
        room_model_matrix = glm::rotate(room_model_matrix, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        room_model_matrix = glm::rotate(room_model_matrix, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        room_model_matrix = glm::scale(room_model_matrix, glm::vec3(1.f, 1.f, 1.f) * 1.f);

        // 添加光源
        for (int i = 0; i < area_lights_position.size(); i++)
        {
            glm::mat4 matrix = glm::mat4(1.0f);
            matrix = glm::rotate(matrix, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            matrix = glm::rotate(matrix, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            matrix = glm::scale(matrix, glm::vec3(1.f, 1.f, 1.f) * 1.0f);
            area_lights_position[i] = glm::vec3(matrix * glm::vec4(area_lights_position[i], 1.0f));
            area_lights_normal[i] = glm::normalize(glm::vec3(matrix * glm::vec4(area_lights_normal[i], 0.0f)));

            light_manager.add_area_light(area_lights_position[i], area_lights_normal[i], 5.0f, 5.0f, glm::vec3(300.0f, 300.0f, 300.0f), 16);
        }

        // 加载着色器
        shader_manager.load_shader("room_shader", "source/shader/classic/room.vs", "source/shader/classic/room.fs");

        this->LoadModels(); // 加载模型
        this->createBLAS(); // 建立低层次的BVH加速结构
        this->createTLAS(); // 建立高层次的BVH加速结构
    }

    // 初始化模型 -- 加载所有的模型
    void ClassicScene::LoadModels()
    {
        std::vector<std::string> modelPaths = {
            "./source/model/shark.obj",
            "./source/model/room/overall.obj"};
        // 先加载所有的模型文件 存储在meshes中
        for (auto path : modelPaths)
        {
            AddModel(path);
        }
    }

    void ClassicScene::render(const glm::mat4 &projection, const glm::mat4 &view, glm::vec3 &camera_pos)
    {
    }
}