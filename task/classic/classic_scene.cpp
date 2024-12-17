#include "classic_scene.hpp"
#include "draw_base_model.hpp"
#include "fluid.hpp"
#include "room.hpp"
#include "butterfly.hpp"
#include "config.hpp"

namespace GL_TASK
{
    ClassicScene::ClassicScene(ShaderManager &shader_manager, LightManager &light_manager, const int WINDOW_WIDTH, const int WINDOW_HEIGHT)
        : Scene(shader_manager, light_manager, WINDOW_WIDTH, WINDOW_HEIGHT)
    {
        setup_scene();
    }

    void ClassicScene::setup_scene()
    {
        // 加载数据
        load_lights();
        load_models();
        // 处理数据
        createBLAS();    // 建立低层次的BVH加速结构
        createTLAS();    // 建立高层次的BVH加速结构
        process_data();  // 将数据转换为传递给着色器的数据形式
        init_GPU_data(); // 将相关数据绑定到纹理中以便传递到GPU中
        init_FBOs();     // 初始化帧缓冲对象
        load_shaders();  // 加载着色器
        // 模拟启动
        if (fluid.get() != nullptr)
            fluid->start(); // 启动流体模拟
        if (bulletWorld.get() != nullptr)
            bulletWorld->start(); // 启动物理模拟
    }

    void ClassicScene::load_shaders()
    {
        // 加载编译着色器
        shader_manager.load_shader("pathTracingShader", "./source/shader/classic/common_vertex_shader.vs", "./source/shader/classic/path_tracing_fragment_shader.fs");
        shader_manager.load_shader("accumulationShader", "./source/shader/classic/common_vertex_shader.vs", "./source/shader/classic/accumulation_fragment_shader.fs");
        shader_manager.load_shader("postProcessShader", "./source/shader/classic/common_vertex_shader.vs", "./source/shader/classic/post_process_fragment_shader.fs");
        shader_manager.load_shader("cubemap_shader", "source/shader/cubemap.vs", "source/shader/cubemap.fs");
        shader_manager.load_shader("liquid_shader", "source/shader/classic/liquid.vs", "source/shader/classic/liquid.fs");

        // 为着色器传递数据
        // auto path_tracing_shader = shader_manager.get_shader("pathTracingShader");
        // path_tracing_shader->use();
        // path_tracing_shader->setInt("accumTexture", 0);
        // path_tracing_shader->setInt("BVHTex", 1);
        // path_tracing_shader->setInt("vertexIndicesTex", 2);
        // path_tracing_shader->setInt("verticesTex", 3);
        // path_tracing_shader->setInt("normalsTex", 4);
        // path_tracing_shader->setInt("transformsTex", 5);
        // path_tracing_shader->setInt("materialsTex", 6);
        // path_tracing_shader->setInt("textureMapsArrayTex", 7);
        // path_tracing_shader->stopUsing();
    }

    void ClassicScene::load_models()
    {
        glm::vec3 roomMin = glm::vec3(-104.160004, -359.567505, -430.721375);
        glm::vec3 roomMax = glm::vec3(104.159973, 77.232498, 99.375420);
        glm::mat4 room_model_matrix = glm::mat4(1.0f);

        // Room model
        printf("/*************************************/\n");
        printf("Load Room Model\n");
        room = std::make_shared<Room>("E:/my_code/GL_bigwork/code/source/model/room2/test.obj", meshes, meshInstances, textures, materials);
        room->getBoundingBox(roomMin, roomMax);
        room_model_matrix = room->get_model_matrix();

        // // butterfly model
        printf("/*************************************/\n");
        printf("Load Butterfly Model\n");
        for (int i = 0; i < butterfly_count; i++)
        {
            auto butterfly_model_single = std::make_shared<Butterfly>("E:/my_code/GL_bigwork/code/source/model/butterfly/ok.dae", meshes, meshInstances, textures, materials);
            butterflies.push_back(butterfly_model_single);
        }

        // liquid model
        printf("/*************************************/\n");
        printf("Load Liquid Model\n");
        fluid = std::make_shared<Fluid>(meshes, meshInstances, textures, materials);
        fluid->BindDirty(&BbvhDirty);
        fluid->set_model_matrix(room_model_matrix);
        fluid->add_model("./source/model/fluid/mesh.obj");

        // bullet world
        bulletWorld = std::make_shared<BulletWorld>(meshes, meshInstances, textures, materials);
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

        // 点云 1
        // printf("/*************************************/\n");
        // printf("Load PointCloud Model\n");
        // glm::mat4 model = glm::mat4(1.0f);
        // model = glm::translate(model, glm::vec3(0.0f, -120.0f, -160.0f));
        // model = glm::scale(model, glm::vec3(0.3f));
        // auto point_cloud1 = std::make_shared<PointCloud>("./source/model/point_cloud/Cumulonimbus_11.vdb", meshes, meshInstances, textures, materials, model);
        // point_clouds.push_back(point_cloud1);
        // 点云 2
        // model = glm::mat4(1.0f);
        // model = glm::translate(model, glm::vec3(0.0f, -300.0f, -110.0f));
        // model = glm::scale(model, glm::vec3(1.0f));
        // auto point_cloud2 = std::make_shared<PointCloud>("./source/model/point_cloud/VDB_PACK_Smoke5.vdb", meshes, meshInstances, textures, materials, model);
        // point_clouds.push_back(point_cloud2);
        //  model = glm::mat4(1.0f);pCM
        //  model = glm::translate(model, glm::vec3(-60.0f, -100.0f, -110.0f));
        //  model = glm::scale(model, glm::vec3(0.4f));
        //  auto point_cloud2 = std::make_shared<PointCloud>("./source/model/point_cloud/Cumulonimbus_14.vdb", meshes, meshInstances, textures, materials, model);
        //  point_clouds.push_back(point_cloud2);
        // 点云 3
        // model = glm::mat4(1.0f);
        // model = glm::translate(model, glm::vec3(40.0f, -140.0f, 0.0f));
        // model = glm::scale(model, glm::vec3(0.5f));
        // auto point_cloud3 = std::make_shared<PointCloud>("./source/model/point_cloud/Cumulonimbus_09.vdb", meshes, meshInstances, textures, materials, model);
        // point_clouds.push_back(point_cloud3);
    }

    void ClassicScene::load_lights()
    {
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
    bool ClassicScene::render_scene(Camera &camera)
    {
        update_scene();
        printf("SampleNumber: %d - FrameNumber:%d\n", sampleNum, frameNum);
        currentBuffer = 1 - currentBuffer;
        if (++sampleNum >= SAMPLES_PER_FRAME)
        {
            // 每帧采样20次
            SaveFrameImage(); // 保存图片
            frameNum++;
            sampleNum = 1;
            // 清除上次路径追踪的渲染结果
            glBindFramebuffer(GL_FRAMEBUFFER, accumFBO);
            glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            return true;
        }

        glActiveTexture(GL_TEXTURE0);
        render_path_tracing(camera);
        render_accumulation();
        render_post_processing();

        return false;
    }

    void ClassicScene::render_path_tracing(Camera &camera)
    {
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
        path_tracing_shader->setVec3("camera.position", camera.get_pos());
        path_tracing_shader->setVec3("camera.up", camera.get_up());
        path_tracing_shader->setVec3("camera.right", camera.get_right());
        path_tracing_shader->setVec3("camera.forward", camera.get_direction());
        path_tracing_shader->setFloat("camera.fov", camera.get_fov());
        path_tracing_shader->setInt("SampleNum", sampleNum); // 设置采样数量
        path_tracing_shader->stopUsing();

        glBindFramebuffer(GL_FRAMEBUFFER, pathTraceFBO);
        glClear(GL_COLOR_BUFFER_BIT);
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
        post_process_shader->setFloat("invSampleCounter", 1.0 / float(this->sampleNum));
        post_process_shader->stopUsing();
        glBindFramebuffer(GL_FRAMEBUFFER, outputFBO);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, outputTexture[currentBuffer], 0);
        glBindTexture(GL_TEXTURE_2D, accumTexture);
        quad->Draw(post_process_shader.get());
    }

    // 将渲染结果输出到屏幕上
    void ClassicScene::present_scene()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
        auto accumulation_shader = shader_manager.get_shader("accumulationShader");
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, outputTexture[1 - currentBuffer]);
        quad->Draw(accumulation_shader.get());
    }

    void ClassicScene::update_models()
    {
        // 更新模型
        printf("/*************************************/\n");
        printf("Update Butterfly Matrix\n");
        for (auto &butterfly : butterflies)
            butterfly->update();
    }

    void ClassicScene::update_scene()
    {
        if (BbvhDirty)
        {
            for (int i = 0; i < meshes.size(); ++i)
            {
                Mesh *mesh = meshes[i];
                if (mesh->needsUpdate(i))
                { // 刷新低层次的BVH加速结构
                    std::cout << "Mesh " << i << " finish an update." << std::endl;
                    is_update = true;
                }
            }
            if (is_update)
            {
                this->update_models();   // 更新模型
                this->createTLAS();      // 重建高层次的BVH加速结构
                this->process_data();    // 处理数据 将其转换成可供Shader使用的形式
                this->update_GPU_data(); // 将相关数据绑定到纹理中以便传递到GPU中
                printf("ClassicScene: A new scene is ready\n");
                is_update = false;
                return; // 不要清除上次路径渲染的结果，等保存完
            }
        }
        if (dirty)
        {
            // 清除上次路径追踪的渲染结果
            glBindFramebuffer(GL_FRAMEBUFFER, accumFBO);
            glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            // 重置帧数和采样数
            sampleNum = 1;
            dirty = false;
        }
    }

    void ClassicScene::wait_until_next_frame(int frame_number)
    {
        if (fluid.get() != nullptr)
            fluid->wait_until_next_frame(frame_number);
    }
}