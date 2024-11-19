
#include "classic_scene.hpp"
#include "draw_base_model.hpp"

namespace GL_TASK
{
    ClassicScene::ClassicScene(ShaderManager &shader_manager, LightManager &light_manager) : Scene(shader_manager, light_manager)
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
            light_manager.add_area_light(area_lights_position[i], glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(30000.0f, 30000.0f, 30000.0f), 5.0f, 5.0f, 16);
        }

        // 加载着色器
        shader_manager.load_shader("room_shader", "source/shader/classic/room.vs", "source/shader/classic/room.fs");
        shader_manager.load_shader("cubemap_shader", "source/shader/cubemap.vs", "source/shader/cubemap.fs");
        shader_manager.load_shader("butterfly_shader", "source/shader/classic/butterfly.vs", "source/shader/classic/butterfly.fs");

        // 创建模型并为每个模型分配着色器
        // room
        /*auto shader = shader_manager.get_shader("room_shader");
        light_manager.apply_lights(shader);
        auto room_model = std::make_shared<Room>("source/model/room/overall.obj", shader, true);
        // auto room_model = std::make_shared<Room>("E:/my_code/GL_bigwork/ToyEffects/ToyEffects/assets/SceneModels/tree1/trees9.obj", shader, true);
        room_model->set_model_matrix(room_model_matrix);
        models.push_back(room_model);*/
        auto b_shader = shader_manager.get_shader("butterfly_shader");
        light_manager.apply_lights(b_shader);
        //auto butterfly_model = std::make_shared<Butterfly>("source/model/butterfly/butterfly-all.dae", b_shader, true);
        for (auto &butterfly_model_matrix : butterfly_model_matrix_vec)
        {
            float scale_rand =  (float)((rand() % (500-200))+ 200)/100;
            float translate_rand = (float)((rand() % (1000-(-1000)))+ (-1000))/100;
            float rotate_rand=(float)((rand() % (900-(-900)))+ (-900))/10;
            //std::cout<<scale_rand<<" "<<translate_rand<<std::endl;
            // butterfly_model_matrix = glm::rotate(butterfly_model_matrix, glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            butterfly_model_matrix = glm::rotate(butterfly_model_matrix, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            butterfly_model_matrix = glm::rotate(butterfly_model_matrix, glm::radians(rotate_rand), glm::vec3(0.0f, 1.0f, 0.0f));
            butterfly_model_matrix = glm::scale(butterfly_model_matrix, glm::vec3(1.f, 1.f, 1.f) * scale_rand);
            butterfly_model_matrix = glm::translate(butterfly_model_matrix, glm::vec3(translate_rand, 0.0f, translate_rand+10.0f));
            //auto butterfly_model_single=butterfly_model;
            //auto butterfly_model_single = std::make_shared<Butterfly>("source/model/butterfly/butterfly-all.dae", b_shader, true);
            //auto butterfly_model_single = std::make_shared<Butterfly>("source/model/butterfly/butterfly1119-2.dae", b_shader, true);
            //auto butterfly_model_single = std::make_shared<Butterfly>("source/model/butterfly/butterfly1119-3.glb", b_shader, true);
            auto butterfly_model_single = std::make_shared<Butterfly>("source/model/butterfly/ehhh.dae", b_shader, true);
            butterfly_model_single->set_model_matrix(butterfly_model_matrix);
            models.push_back(butterfly_model_single);
        }
    }

    void ClassicScene::render(const glm::mat4 &projection, const glm::mat4 &view, glm::vec3 &camera_pos)
    {

        for (const auto &model : models)
        {
            model->draw(projection, view, camera_pos);
        }

        /// 调试用
        auto shader = shader_manager.get_shader("cubemap_shader");
        shader->use();
        shader->setMat4("projection", projection);
        shader->setMat4("view", view);
        render_sphere();
        for (auto &po : area_lights_position)
        {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, po);
            shader->setMat4("model", model);
            render_sphere();
        }
        ///
    }
}