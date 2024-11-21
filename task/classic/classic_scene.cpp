
#include "classic_scene.hpp"
#include "draw_base_model.hpp"

namespace GL_TASK
{
    ClassicScene::ClassicScene(ShaderManager &shader_manager, LightManager &light_manager, float precision) : Scene(shader_manager, light_manager), precision(precision)
    {
        setup_scene();
    }

    void ClassicScene::setup_scene()
    {
        room_model_matrix = glm::rotate(room_model_matrix, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        room_model_matrix = glm::rotate(room_model_matrix, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        room_model_matrix = glm::scale(room_model_matrix, glm::vec3(1.f, 1.f, 1.f) * 1.f);

        // 添加光源
        for (int i = 0; i < area_lights_position.size(); i++)
        {
            // printf("position: %f %f %f\n", area_lights_position[i].x, area_lights_position[i].y, area_lights_position[i].z);
            light_manager.add_area_light(area_lights_position[i], glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f) * 30000.0f, 4.0f, 4.0f, 4);
        }

        // 加载着色器
        shader_manager.load_shader("room_shader", "source/shader/classic/room.vs", "source/shader/classic/room.fs");
        shader_manager.load_shader("cubemap_shader", "source/shader/cubemap.vs", "source/shader/cubemap.fs");
        shader_manager.load_shader("cloud", "source/shader/point_cloud.vs", "source/shader/point_cloud.fs");
        shader_manager.load_shader("liquid_shader", "source/shader/classic/liquid.vs", "source/shader/classic/liquid.fs");

        // room
        auto shader = shader_manager.get_shader("room_shader");
        light_manager.apply_lights(shader);
        auto room_model = std::make_shared<Room>("source/model/room/overall.obj", shader, true);
        room_model->set_model_matrix(room_model_matrix);
        models.push_back(room_model);

        // 调试在线渲染请注释掉水模型，否则会非常卡
        // Liquid model
        // auto liquid_shader = shader_manager.get_shader("liquid_shader");
        // light_manager.apply_lights(liquid_shader);
        // glm::mat4 liquid_model_matrix = glm::scale(room_model_matrix, glm::vec3(1.f, 1.f, 1.f) * (1.f / precision)); // Adjust scale
        // auto liquid_model = std::make_shared<Room>("source/model/fluid/mesh.obj", liquid_shader, true);
        // liquid_model->set_model_matrix(liquid_model_matrix);
        // models.push_back(liquid_model);

        // 点云
        auto cloud_shader1 = shader_manager.get_shader("cloud");
        auto point_cloud1 = std::make_shared<PointCloud>("source/model/point_cloud/Cumulonimbus_11.vdb", cloud_shader1);
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 30.0f, -30.0f));
        model = glm::scale(model, glm::vec3(0.3f));
        point_cloud1->set_model_matrix(model);
        point_clouds.push_back(point_cloud1);

        auto cloud_shader2 = shader_manager.get_shader("cloud");
        auto point_cloud2 = std::make_shared<PointCloud>("source/model/point_cloud/Cumulonimbus_14.vdb", cloud_shader2);
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-80.0f, 40.0f, -110.0f));
        model = glm::scale(model, glm::vec3(0.4f));
        point_cloud2->set_model_matrix(model);
        point_clouds.push_back(point_cloud2);
    }

    void ClassicScene::render(const glm::mat4 &projection, const glm::mat4 &view, glm::vec3 &camera_pos)
    {
        for (const auto &model : models)
        {
            model->draw(projection, view, camera_pos);
        }

        glDepthMask(GL_FALSE); // 禁止深度写入
        for (const auto &point_cloud : point_clouds)
        {
            point_cloud->draw(projection, view, camera_pos, glm::vec3(0.9f, 0.9f, 0.9f), 0.9f, false); // 在线框模式下不要深度排序
        }
        glDepthMask(GL_TRUE); // 重新启用深度写入

        // /// 调试用
        // auto shader = shader_manager.get_shader("cubemap_shader");
        // shader->use();
        // shader->setMat4("projection", projection);
        // shader->setMat4("view", view);
        // render_sphere();
        // for (auto &po : area_lights_position)
        // {
        //     glm::mat4 model = glm::mat4(1.0f);
        //     model = glm::translate(model, po);
        //     shader->setMat4("model", model);
        //     render_sphere();
        // }
    }
}