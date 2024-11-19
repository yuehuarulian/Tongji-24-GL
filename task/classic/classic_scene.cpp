#include "classic_scene.hpp"
#include "draw_base_model.hpp"
#include "bullet_handler.hpp"
#include "room.hpp"
#include "boat.hpp"
#include "fluid.hpp"

namespace GL_TASK
{
    ClassicScene::ClassicScene(ShaderManager &shader_manager, LightManager &light_manager, float precision)
        : Scene(shader_manager, light_manager), precision(precision)
    {
        setup_scene();
    }

    ClassicScene::~ClassicScene()
    {
        cleanup_physics(dynamicsWorld);
    }

    void ClassicScene::setup_scene()
    {
        // 添加光源
        for (size_t i = 0; i < area_lights_position.size(); ++i)
            light_manager.add_area_light(area_lights_position[i], area_lights_normal[i], glm::vec3(1.0f) * 30000.0f, 4.0f, 4.0f, 4);

        // 加载着色器
        shader_manager.load_shader("room_shader", "source/shader/classic/room.vs", "source/shader/classic/room.fs");
        shader_manager.load_shader("cubemap_shader", "source/shader/cubemap.vs", "source/shader/cubemap.fs");
        shader_manager.load_shader("cloud", "source/shader/point_cloud.vs", "source/shader/point_cloud.fs");
        shader_manager.load_shader("liquid_shader", "source/shader/classic/liquid.vs", "source/shader/classic/liquid.fs");

        // room
        auto shader = shader_manager.get_shader("room_shader");
        light_manager.apply_lights(shader);
        auto room_model = std::make_shared<Room>("source/model/room/overall.obj", shader, true);
        // 设置房间模型
        glm::mat4 room_model_matrix = glm::mat4(1.0f);
        room_model_matrix = glm::rotate(room_model_matrix, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        room_model_matrix = glm::rotate(room_model_matrix, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        room_model_matrix = glm::scale(room_model_matrix, glm::vec3(1.f, 1.f, 1.f) * 1.f);
        room_model->set_model_matrix(room_model_matrix);
        models.push_back(room_model);

        // 调试在线渲染请注释掉水模型，否则会非常卡
        // Liquid model
        auto liquid_shader = shader_manager.get_shader("liquid_shader");
        light_manager.apply_lights(liquid_shader);
        glm::mat4 liquid_model_matrix = glm::scale(room_model_matrix, glm::vec3(1.f, 1.f, 1.f) * (1.f / precision)); // Adjust scale
        auto liquid_model = std::make_shared<Fluid>("source/model/fluid/mesh.obj", liquid_shader, true);
        liquid_model->set_model_matrix(liquid_model_matrix);
        models.push_back(liquid_model);

        // 加载船模型
        auto boat_shader = shader_manager.get_shader("boat_shader");
        light_manager.apply_lights(boat_shader);
        boatInstance = std::make_shared<Boat>("source/model/boat/boat_obj.obj", boat_shader, true);
        glm::mat4 boat_model_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 5.0f, 0.0f));
        boat_model_matrix = glm::scale(boat_model_matrix, glm::vec3(5.f));
        boatInstance->set_model_matrix(boat_model_matrix);
        models.push_back(boatInstance);

        // 初始化 Bullet 物理引擎
        dynamicsWorld = initBullet(boatInstance.get());
    }

    void ClassicScene::render(const glm::mat4 &projection, const glm::mat4 &view, glm::vec3 &camera_pos)
    {
        auto fluidInstance = std::dynamic_pointer_cast<Fluid>(models[1]);
        if (!fluidInstance)
            throw std::runtime_error("models[1] is not a Fluid instance.");
        applyFluidForces(boatInstance->getRigidBody(), fluidInstance);

        // glm::mat4 boatTransform = computeBoatTransformFromPhysics();
        // boatModel->set_model_matrix(boatTransform);

        // 渲染模型
        for (const auto &model : models)
        {
            model->draw(projection, view, camera_pos);
        }

        glDepthMask(GL_FALSE); // 禁止深度写入
        for (const auto &point_cloud : point_clouds)
        {
            point_cloud->draw(projection, view, camera_pos, glm::vec3(0.9f, 0.9f, 0.9f), 0.9f, false); // 在线框模式下不要深度排序
        }
        glDepthMask(GL_TRUE);
    }

}
