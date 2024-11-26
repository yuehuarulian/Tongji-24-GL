#include "classic_scene.hpp"
#include "draw_base_model.hpp"

namespace GL_TASK
{
    ClassicScene::ClassicScene(ShaderManager &shader_manager, LightManager &light_manager) : Scene(shader_manager, light_manager)
    {
        setup_scene(); // 配置场景
        InitGPUData(); // 将相关数据绑定到纹理中以便传递到GPU中
        InitFBOs();    // 初始化帧缓冲对象
        InitShaders(); // 将相关数据传递到Shader中
    }

    // 对场景进行设置
    void ClassicScene::setup_scene()
    {
        frameNum = 1;
        LoadLights(); // 加载光源数据
        LoadModels(); // 加载模型数据
    }

    void ClassicScene::InitShaders()
    {
        // 路径追踪着色器
        shader_manager.load_shader("pathTracingShader",
                                   "./source/shader/classic/common_vertex_shader.vs",
                                   "./source/shader/classic/path_tracing_fragment_shader.fs");
        // 累积结果着色器
        shader_manager.load_shader("accumulationShader",
                                   "./source/shader/classic/common_vertex_shader.vs",
                                   "./source/shader/classic/accumulation_fragment_shader.fs");
        // 后处理着色器
        shader_manager.load_shader("postProcessShader",
                                   "./source/shader/classic/common_vertex_shader.vs",
                                   "./source/shader/classic/post_process_fragment_shader.fs");

        auto path_tracing_shader = shader_manager.get_shader("pathTracingShader");
        path_tracing_shader->use();
        path_tracing_shader->setInt("topBVHIndex", bvhConverter.topLevelIndex);
        path_tracing_shader->setVec2("resolution", 1080, 720);

        path_tracing_shader->setInt("accumTexture", 0);
        path_tracing_shader->setInt("BVH", 1);
        path_tracing_shader->setInt("vertexIndicesTex", 2);
        path_tracing_shader->setInt("verticesTex", 3);
        path_tracing_shader->setInt("normalsTex", 4);
        path_tracing_shader->setInt("transformsTex", 5);
        path_tracing_shader->setInt("materialsTex", 6);
        path_tracing_shader->setInt("textureMapsArrayTex", 7);

        path_tracing_shader->stopUsing();
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

    void ClassicScene::updateCameraInfo(Camera *camera)
    {
        glm::vec3 cameraPos = camera->get_pos();
        auto path_tracing_shader = shader_manager.get_shader("pathTracingShader");
        path_tracing_shader->use();
        path_tracing_shader->setVec3("camera.position", cameraPos.x, cameraPos.y, cameraPos.z);
        path_tracing_shader->setVec3("camera.up", camera->get_up());
        path_tracing_shader->setVec3("camera.right", camera->get_right());
        path_tracing_shader->setVec3("camera.forward", -camera->get_direction());
        path_tracing_shader->setFloat("camera.fov", camera->get_fov());
        path_tracing_shader->stopUsing();
    }

    /*
     * 渲染函数
     * 负责在多个帧缓冲对象之间绘制图形，并更新渲染结果
     */
    void ClassicScene::render()
    {
        auto path_tracing_shader = shader_manager.get_shader("pathTracingShader");
        auto accumulation_shader = shader_manager.get_shader("accumulationShader");
        auto post_process_shader = shader_manager.get_shader("postProcessShader");

        path_tracing_shader->use();
        path_tracing_shader->setInt("frameNum", this->frameNum);
        path_tracing_shader->stopUsing();

        post_process_shader->use();
        post_process_shader->setFloat("invSampleCounter", 1.0 / float(this->frameNum));
        post_process_shader->stopUsing();

        // 激活纹理单元 0，用于绑定帧缓冲对象的目标纹理
        glActiveTexture(GL_TEXTURE0);

        // 1. 路径追踪阶段
        // 绑定路径追踪帧缓冲对象并设置视口
        glBindFramebuffer(GL_FRAMEBUFFER, pathTraceFBO);
        glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
        // 绑定累积纹理，作为路径追踪的输入数据
        glBindTexture(GL_TEXTURE_2D, accumTexture);
        // 使用路径追踪着色器渲染
        quad->Draw(path_tracing_shader.get());

        // 2. 累积阶段
        // 绑定累积帧缓冲对象并设置视口
        glBindFramebuffer(GL_FRAMEBUFFER, accumFBO);
        glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
        // 绑定路径追踪纹理，作为累积阶段的输入
        glBindTexture(GL_TEXTURE_2D, pathTraceTexture);
        // 使用累积着色器进行渲染
        quad->Draw(accumulation_shader.get());

        // 3. 后期处理阶段
        // 绑定输出帧缓冲对象并设置目标纹理
        glBindFramebuffer(GL_FRAMEBUFFER, outputFBO);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, outputTexture[currentBuffer], 0);
        // 设置视口大小
        glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
        // 绑定累积纹理，作为后期处理的输入
        glBindTexture(GL_TEXTURE_2D, accumTexture);
        // 使用后期处理着色器渲染最终结果
        quad->Draw(post_process_shader.get());
    }

    /*
     * 显示函数
     * 将渲染结果输出到屏幕上
     */
    void ClassicScene::present()
    {
        auto accumulation_shader = shader_manager.get_shader("accumulationShader");
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, outputTexture[1 - currentBuffer]);
        quad->Draw(accumulation_shader.get());
    }

    void ClassicScene::update()
    {
        if (dirty)
        {
            frameNum = 1;
            glBindFramebuffer(GL_FRAMEBUFFER, accumFBO);
            glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
            glClear(GL_COLOR_BUFFER_BIT);
        }
        else
        {
            frameNum++;
            currentBuffer = 1 - currentBuffer;
        }
    }
}