
#include "classic_scene.hpp"
#include "draw_base_model.hpp"
#include "room.hpp"
#include "fluid.hpp"
#include "butterfly.hpp"

namespace GL_TASK
{
    ClassicScene::ClassicScene(ShaderManager &shader_manager, LightManager &light_manager) : Scene(shader_manager, light_manager)
    {
        setup_scene();
    }

    void ClassicScene::setup_scene()
    {
        // 添加光源
        for (int i = 0; i < area_lights_position.size(); i++)
            light_manager.add_area_light(area_lights_position[i], glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f) * 30000.0f, 4.0f, 4.0f, 4);

        // 加载着色器
        shader_manager.load_shader("room_shader", "source/shader/classic/room.vs", "source/shader/classic/room.fs");
        shader_manager.load_shader("cubemap_shader", "source/shader/cubemap.vs", "source/shader/cubemap.fs");
        shader_manager.load_shader("cloud", "source/shader/point_cloud.vs", "source/shader/point_cloud.fs");
        shader_manager.load_shader("liquid_shader", "source/shader/classic/liquid.vs", "source/shader/classic/liquid.fs");
        shader_manager.load_shader("butterfly_shader", "source/shader/classic/butterfly.vs", "source/shader/classic/butterfly.fs");

        // room
        auto shader = shader_manager.get_shader("room_shader");
        light_manager.apply_lights(shader);
        auto room_model = std::make_shared<Room>("source/model/room1.0/overall.obj", shader, true);
        glm::mat4 room_model_matrix = glm::mat4(1.0f);
        room_model_matrix = glm::rotate(room_model_matrix, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        room_model_matrix = glm::rotate(room_model_matrix, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        room_model_matrix = glm::scale(room_model_matrix, glm::vec3(1.f, 1.f, 1.f) * 1.3f);
        room_model->set_model_matrix(room_model_matrix);
        models.push_back(room_model);

        // // Liquid model  调试在线渲染请注释掉水模型，否则会非常卡
        // auto liquid_shader = shader_manager.get_shader("liquid_shader");
        // light_manager.apply_lights(liquid_shader);
        // // glm::mat4 liquid_model_matrix = glm::scale(room_model_matrix, glm::vec3(1.f, 1.f, 1.f) * (1.f / precision)); // Adjust scale
        // auto liquid_model = std::make_shared<Fluid>("source/model/fluid/mesh.obj", liquid_shader, true);
        // liquid_model->set_model_matrix(room_model_matrix);
        // models.push_back(liquid_model);

        // butterfly
        auto b_shader = shader_manager.get_shader("butterfly_shader");
        light_manager.apply_lights(b_shader);
        for (auto &butterfly_model_matrix : butterfly_model_matrix_vec)
        {
            float scale_rand = (float)((rand() % (500 - 200)) + 200) / 100 * 5;

            float translate_rand = (float)((rand() % (1000 - (-1000))) + (-1000)) / 100;
            float rotate_rand = (float)((rand() % (900 - (-900))) + (-900)) / 10;
            butterfly_model_matrix = glm::rotate(butterfly_model_matrix, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            butterfly_model_matrix = glm::rotate(butterfly_model_matrix, glm::radians(rotate_rand), glm::vec3(0.0f, 1.0f, 0.0f));
            butterfly_model_matrix = glm::scale(butterfly_model_matrix, glm::vec3(1.f, 1.f, 1.f) * scale_rand);
            butterfly_model_matrix = glm::translate(butterfly_model_matrix, glm::vec3(translate_rand, 0.0f, translate_rand + 10.0f));
            auto butterfly_model_single = std::make_shared<Butterfly>("source/model/butterfly/ehhh.dae", b_shader, true);
            butterfly_model_single->set_model_matrix(butterfly_model_matrix);
            models.push_back(butterfly_model_single);
        }

        // // 点云
        // auto cloud_shader1 = shader_manager.get_shader("cloud");
        // auto point_cloud1 = std::make_shared<PointCloud>("source/model/point_cloud/Cumulonimbus_11.vdb", cloud_shader1);
        // glm::mat4 model = glm::mat4(1.0f);
        // model = glm::translate(model, glm::vec3(0.0f, -20.0f, -30.0f));
        // model = glm::scale(model, glm::vec3(0.4f));
        // point_cloud1->set_model_matrix(model);
        // point_clouds.push_back(point_cloud1);

        // auto cloud_shader2 = shader_manager.get_shader("cloud");
        // auto point_cloud2 = std::make_shared<PointCloud>("source/model/point_cloud/Cumulonimbus_14.vdb", cloud_shader2);
        // model = glm::mat4(1.0f);
        // model = glm::translate(model, glm::vec3(-80.0f, -40.0f, -110.0f));
        // model = glm::scale(model, glm::vec3(0.4f));
        // point_cloud2->set_model_matrix(model);
        // point_clouds.push_back(point_cloud2);

        // auto cloud_shader3 = shader_manager.get_shader("cloud");
        // auto point_cloud3 = std::make_shared<PointCloud>("source/model/point_cloud/Cumulonimbus_09.vdb", cloud_shader3);
        // model = glm::mat4(1.0f);
        // model = glm::translate(model, glm::vec3(40.0f, -80.0f, 40.0f));
        // model = glm::scale(model, glm::vec3(0.5f));
        // point_cloud3->set_model_matrix(model);
        // point_clouds.push_back(point_cloud3);
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
            point_cloud->draw(projection, view, camera_pos, glm::vec3(0.9f, 0.9f, 0.9f), 0.9f, false, true, 1 / 30.); // 在线框模式下不要深度排序
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

    void ClassicScene::wait_until_next_frame(int frame_number)
    {
        auto fluid = std::dynamic_pointer_cast<Fluid>(models[1]);
        fluid->wait_until_next_frame(frame_number);
    }
}