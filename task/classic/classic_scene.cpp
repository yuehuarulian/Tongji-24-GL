#include "classic_scene.hpp"
#include "draw_base_model.hpp"

namespace GL_TASK
{
    ClassicScene::ClassicScene(ShaderManager &shader_manager, LightManager &light_manager) : Scene(shader_manager, light_manager)
    {
        setup_scene(); // 配置场景
        InitGPUData(); // 将相关数据绑定到纹理中以便传递到GPU中
        InitShaders(); // 将相关数据传递到Shader中
    }

    // 对场景进行设置
    void ClassicScene::setup_scene()
    {
        LoadLights(); // 加载光源数据
        LoadModels(); // 加载模型数据
    }

    // 初始化模型 -- 加载所有的模型
    void ClassicScene::LoadModels()
    {
        // 模型文件路径
        std::vector<std::string> modelPaths = {
            // "./source/model/shark.obj",
            "./source/model/room/overall.obj",
            // "./source/model/nanosuit/nanosuit.obj"
        };
        glm::mat4 room_model_matrix = glm::mat4(1.0f);
        room_model_matrix = glm::rotate(room_model_matrix, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        room_model_matrix = glm::rotate(room_model_matrix, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        room_model_matrix = glm::scale(room_model_matrix, glm::vec3(1.f, 1.f, 1.f) * 1.3f);
        // 先加载所有的模型文件 存储在meshes中
        for (auto path : modelPaths)
            AddModel(path, room_model_matrix);

        this->createBLAS();  // 建立低层次的BVH加速结构
        this->createTLAS();  // 建立高层次的BVH加速结构
        this->ProcessData(); // 处理数据 将其转换成可供Shader使用的形式
    }
    // 初始化光源 -- 加载所有的光源
    void ClassicScene::LoadLights()
    {
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
    }

    void ClassicScene::InitShaders()
    {
        // 加载着色器
        shader_manager.load_shader("room_shader", "source/shader/classic/room.vs", "source/shader/classic/room.fs");
        shader_manager.load_shader("tile_shader", "source/shader/common_vertex_shader.vs", "source/shader/tile_fragment_shader.fs");

        Shader *shaderObject = shader_manager.get_shader("tile_shader").get();
        shaderObject->use();
        // 传递数据到Shader中
        shaderObject->setInt("topBVHIndex", bvhConverter.topLevelIndex);
        shaderObject->setInt("BVH", 1);
        shaderObject->setInt("vertexIndicesTex", 2);
        shaderObject->setInt("verticesTex", 3);
        shaderObject->setInt("normalsTex", 4);
        shaderObject->setInt("transformsTex", 5);
        shaderObject->setInt("materialsTex", 6);
        shaderObject->setInt("textureMapsArrayTex", 7);
        shaderObject->setVec2("resolution", 1080, 720);
        shaderObject->stopUsing();
    }

    void ClassicScene::updateCameraInfo(Camera *camera)
    {
        glm::vec3 cameraPos = camera->get_pos();
        auto shaderObject = shader_manager.get_shader("tile_shader");
        shaderObject->use();
        shaderObject->setVec3("camera.position", cameraPos.x, cameraPos.y, cameraPos.z);
        shaderObject->setVec3("camera.up", camera->get_up());
        shaderObject->setVec3("camera.right", camera->get_right());
        shaderObject->setVec3("camera.forward", -camera->get_direction());
        shaderObject->setFloat("camera.fov", camera->get_fov());
        shaderObject->stopUsing();
    }

    void ClassicScene::render()
    {
        std::shared_ptr<Shader> shaderObject = shader_manager.get_shader("tile_shader");
        quad->Draw(shaderObject.get());
    }
}