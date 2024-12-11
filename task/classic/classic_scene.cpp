#include "classic_scene.hpp"
#include "draw_base_model.hpp"
#include "fluid.hpp"
#include "room.hpp"
#include "butterfly.hpp"

namespace GL_TASK
{
    ClassicScene::ClassicScene(ShaderManager &shader_manager, LightManager &light_manager, const int WINDOW_WIDTH, const int WINDOW_HEIGHT)
        : Scene(shader_manager, light_manager, WINDOW_WIDTH, WINDOW_HEIGHT)
    {
        setup_scene(); // 配置场景
    }

    // 对场景进行设置
    void ClassicScene::setup_scene()
    {
        frameNum = 1;
        load_lights();
        load_models();
        init_GPU_data(); // 将相关数据绑定到纹理中以便传递到GPU中
        init_FBOs();     // 初始化帧缓冲对象
        load_shaders();
    }

    void ClassicScene::load_shaders()
    {
        shader_manager.load_shader("pathTracingShader", "./source/shader/classic/common_vertex_shader.vs", "./source/shader/classic/path_tracing_fragment_shader.fs");
        shader_manager.load_shader("accumulationShader", "./source/shader/classic/common_vertex_shader.vs", "./source/shader/classic/accumulation_fragment_shader.fs");
        shader_manager.load_shader("postProcessShader", "./source/shader/classic/common_vertex_shader.vs", "./source/shader/classic/post_process_fragment_shader.fs");
        shader_manager.load_shader("cubemap_shader", "source/shader/cubemap.vs", "source/shader/cubemap.fs");
        shader_manager.load_shader("cloud", "source/shader/point_cloud.vs", "source/shader/point_cloud.fs");
        shader_manager.load_shader("liquid_shader", "source/shader/classic/liquid.vs", "source/shader/classic/liquid.fs");
        // shader_manager.load_shader("butterfly_shader", "source/shader/classic/butterfly.vs", "source/shader/classic/butterfly.fs");

        auto path_tracing_shader = shader_manager.get_shader("pathTracingShader");
        path_tracing_shader->use();
        path_tracing_shader->setInt("topBVHIndex", bvhConverter.topLevelIndex);
        path_tracing_shader->setVec2("resolution", 1080, 720);
        path_tracing_shader->setInt("accumTexture", 0);
        path_tracing_shader->setInt("BVHTex", 1);
        path_tracing_shader->setInt("vertexIndicesTex", 2);
        path_tracing_shader->setInt("verticesTex", 3);
        path_tracing_shader->setInt("normalsTex", 4);
        path_tracing_shader->setInt("transformsTex", 5);
        path_tracing_shader->setInt("materialsTex", 6);
        path_tracing_shader->setInt("textureMapsArrayTex", 7);
        path_tracing_shader->stopUsing();
    }

    void ClassicScene::load_models()
    {
        // 先加载所有的模型文件 存储在meshes中
        // Room model
        glm::vec3 roomMin, roomMax;
        room = std::make_shared<Room>("source/model/room2/room2.obj", meshes, meshInstances, textures, materials);
        printf("Load Room Model Over\n");
        room->getBoundingBox(roomMin, roomMax);
        roomMin = glm::vec3(-104.160004, -359.567505, -430.721375);
        roomMax = glm::vec3(104.159973, 77.232498, 99.375420);

        // liquid model
        glm::mat4 room_model_matrix = glm::mat4(1.0f); // room->get_model_matrix(); // 
        room_model_matrix = glm::rotate(room_model_matrix, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        room_model_matrix = glm::scale(room_model_matrix, glm::vec3(1.f, 1.f, 1.f) * 1.3f);
        fluid = std::make_shared<Fluid>(meshes, meshInstances, textures, materials);
        fluid->BindDirty(&BbvhDirty);
        fluid->set_model_matrix(room_model_matrix);
        fluid->add_model("source/model/fluid/mesh.obj");

        // bullet world
        bulletWorld = std::make_shared<BulletWorld>(meshes, meshInstances, textures, materials);
        bulletWorld->BindDirty(&TbvhDirty);
        bulletWorld->BindFluid(fluid);
        bulletWorld->setRoomBounds(roomMin, roomMax);
        double water_level = fluid->get_water_level(roomMin, roomMax);

        // boat model
        bulletWorld->bind_model("source/model/boat/boat_obj.obj", ObjectType::BOAT);
        bulletWorld->add_model(glm::vec3(15.0, water_level + 5.0, -25.0));
        bulletWorld->add_model(glm::vec3(-15.0, water_level + 5.0, 25.0));

        // flower model
        bulletWorld->bind_model("source/model/flower/flower.obj", ObjectType::FLOWER);
        bulletWorld->add_model(glm::vec3(-20.0, water_level + 2.0, -50.0));
        bulletWorld->add_model(glm::vec3(20.0, water_level + 2.0, 50.0));

        // butterfly model
        // for (int i = 0; i < butterfly_count; i++)
        //{
        // auto butterfly_model_single = std::make_shared<Butterfly>("source/model/butterfly/ok.dae", meshes, meshInstances, textures, materials);
        // butterflies.push_back(butterfly_model_single);
        //}

        this->createBLAS();   // 建立低层次的BVH加速结构
        this->createTLAS();   // 建立高层次的BVH加速结构
        this->process_data(); // 处理数据 将其转换成可供Shader使用的形式
        fluid->start();       // 启动流体模拟
        bulletWorld->start(); // 启动物理模拟

        // 点云
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

    void ClassicScene::load_lights()
    {
        // 添加光源
        for (int i = 0; i < area_lights_position.size(); i++)
        {
            glm::mat4 matrix = glm::mat4(1.0f);
            matrix = glm::rotate(matrix, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            matrix = glm::rotate(matrix, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            matrix = glm::scale(matrix, glm::vec3(1.f, 1.f, 1.f) * 1.0f);
            area_lights_position[i] = glm::vec3(matrix * glm::vec4(area_lights_position[i], 1.0f));
            area_lights_normal[i] = glm::normalize(glm::vec3(matrix * glm::vec4(area_lights_normal[i], 0.0f)));

            light_manager.add_area_light(area_lights_position[i], area_lights_normal[i], glm::vec3(300.0f, 300.0f, 300.0f), 5.0f, 5.0f, 16);
        }
    }

    // 渲染
    void ClassicScene::render(Camera &camera)
    {
        glActiveTexture(GL_TEXTURE0);
        render_path_tracing(camera);
        render_accumulation();
        render_post_processing();
        // render_point_clouds(camera);
    }

    void ClassicScene::render_path_tracing(Camera &camera)
    {
        auto path_tracing_shader = shader_manager.get_shader("pathTracingShader");
        path_tracing_shader->use();
        path_tracing_shader->setVec3("camera.position", camera.get_pos());
        path_tracing_shader->setVec3("camera.up", camera.get_up());
        path_tracing_shader->setVec3("camera.right", camera.get_right());
        path_tracing_shader->setVec3("camera.forward", camera.get_direction());
        path_tracing_shader->setFloat("camera.fov", camera.get_fov());
        path_tracing_shader->setInt("frameNum", frameNum);
        path_tracing_shader->stopUsing();

        glBindFramebuffer(GL_FRAMEBUFFER, pathTraceFBO);
        glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
        glBindTexture(GL_TEXTURE_2D, accumTexture);
        quad->Draw(path_tracing_shader.get());
    }

    void ClassicScene::render_accumulation()
    {
        auto shader = shader_manager.get_shader("accumulationShader");
        glBindFramebuffer(GL_FRAMEBUFFER, accumFBO);
        glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
        glBindTexture(GL_TEXTURE_2D, pathTraceTexture);
        quad->Draw(shader.get());
    }

    void ClassicScene::render_post_processing()
    {
        auto post_process_shader = shader_manager.get_shader("postProcessShader");
        post_process_shader->use();
        post_process_shader->setFloat("invSampleCounter", 1.0 / float(this->frameNum));
        post_process_shader->stopUsing();
        glBindFramebuffer(GL_FRAMEBUFFER, outputFBO);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, outputTexture[currentBuffer], 0);
        glBindTexture(GL_TEXTURE_2D, accumTexture);
        quad->Draw(post_process_shader.get());
    }

    void ClassicScene::render_point_clouds(Camera &camera)
    {
        glDepthMask(GL_FALSE);
        for (const auto &point_cloud : point_clouds)
        {
            point_cloud->draw(camera.projection, camera.view, camera.get_pos(), glm::vec3(0.9f), 0.9f, false, true, 1 / 30.);
        }
        glDepthMask(GL_TRUE);
    }

    // 将渲染结果输出到屏幕上
    void ClassicScene::present()
    {
        auto accumulation_shader = shader_manager.get_shader("accumulationShader");
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, outputTexture[1 - currentBuffer]);
        // this->DenoiseProcess();                        // 进行降噪处理
        // glBindTexture(GL_TEXTURE_2D, denoisedTexture); // 降噪之后的纹理
        quad->Draw(accumulation_shader.get());
    }

    void ClassicScene::update()
    {
        static int current_buffer = 0;
        current_buffer++;
        if (current_buffer % 3 == 0)
        {
            for (auto &butterfly : butterflies)
                butterfly->update();
            if (butterflies.size() > 0)
                TbvhDirty = true;
        }
        if (BbvhDirty || TbvhDirty)
        {
            dirty = true;
            for (int i = 0; i < meshes.size(); ++i)
            {
                Mesh *mesh = meshes[i];
                if (mesh->needsUpdate(i))
                { // 刷新低层次的BVH加速结构
                    std::cout << "Mesh " << i << " finish an update." << std::endl;
                    BbvhDirty = false;
                }
            }
            if (!BbvhDirty || TbvhDirty)
            {
                if (TbvhDirty)
                {
                    this->createTLAS(); // 重建高层次的BVH加速结构
                    TbvhDirty = false;
                }
                this->process_data();    // 处理数据 将其转换成可供Shader使用的形式
                this->update_GPU_data(); // 将相关数据绑定到纹理中以便传递到GPU中
                this->update_FBOs();     // 重置帧缓冲对象
                printf("ClassicScene: A new scene is ready\n");
            }
        }
        if (dirty)
        {
            // printf("Scene is dirty\n");
            frameNum = 1;
            glBindFramebuffer(GL_FRAMEBUFFER, accumFBO);
            glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            // TODO: 数据修改与传输
        }
        else
        {
            // printf("Scene isn't dirty\n");
            frameNum++;
            currentBuffer = 1 - currentBuffer;
        }
    }

    void ClassicScene::wait_until_next_frame(int frame_number)
    {
        fluid->wait_until_next_frame(frame_number);
    }
}
