#include "render_manager.hpp"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include "config.hpp"
#include <iostream>
#include <vector>
#include <sstream>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <filesystem>

RenderManager::RenderManager(int window_width, int window_height, int frames)
    : window_width(window_width), window_height(window_height), frames(frames), offscreen(false) {}

RenderManager::~RenderManager()
{
    if (offscreen)
    {
        // glDeleteFramebuffers(1, &msaa_fbo);
        // glDeleteTextures(1, &msaa_texture);
        // glDeleteRenderbuffers(1, &msaa_rbo);
        // glDeleteFramebuffers(1, &resolve_fbo);
        // glDeleteTextures(1, &resolve_texture);
    }
    if (window)
        glfwDestroyWindow(window);

    glfwTerminate();
}

void RenderManager::initialize()
{
    initialize_GLFW();

    camera = Camera(window, 75 * D2R, glm::vec3(0.0f, -180.0f, 80.0f), glm::pi<float>(), 0. * D2R, 30.0f, 1.0f);
    scene = std::make_unique<GL_TASK::ClassicScene>(shader_manager, light_manager); // TODO
    skybox = std::make_unique<Skybox>(faces, "source/shader/skybox.vs", "source/shader/skybox.fs");

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDepthFunc(GL_LESS);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD); // 增加混合的控制
}

void RenderManager::initialize_GLFW()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, offscreen ? GLFW_FALSE : GLFW_TRUE);

    window = glfwCreateWindow(window_width, window_height, "Scene", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glViewport(0, 0, window_width, window_height);

    // if (offscreen)
    //     initialize_framebuffer();
}

void RenderManager::start_rendering(bool offscreen)
{
    this->offscreen = offscreen;
    initialize();
    if (CAMERA_ANIMATION)
    {
        // 打开文件读取相机动画
        std::ifstream camera_file("E:/my_code/GL_bigwork/code/source/camera_path/camera_transforms.txt");
        if (!camera_file.is_open())
        {
            std::cerr << "Unable to open file for reading camera animation\n";
            return;
        }

        // 读取旋转矩阵和位移数据
        // | R11 R12 R13 Tx |
        // | R21 R22 R23 Ty |
        // | R31 R32 R33 Tz |
        // |  0   0   0  1 |
        camera_transforms.clear(); // 清空之前的路径数据
        float M[16];               // 用于存储 4x4 变换矩阵的 16 个值

        glm::mat4 room_matrix = glm::mat4(1.0f);
        room_matrix = glm::rotate(room_matrix, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        room_matrix = glm::scale(room_matrix, glm::vec3(1.f, 1.f, 1.f) * 1.3f);

        while (camera_file)
        {
            // 读取 4x4 变换矩阵数据
            for (int i = 0; i < 16; ++i)
                camera_file >> M[i];

            if (camera_file)
            {
                // 使用 glm::mat4 来表示 4x4 变换矩阵
                glm::mat4 transform_matrix(
                    M[0], M[1], M[2], M[3],
                    M[4], M[5], M[6], M[7],
                    M[8], M[9], M[10], M[11],
                    M[12], M[13], M[14], M[15]);

                camera_transforms.push_back(room_matrix * transform_matrix); // 将每一帧的变换矩阵存储
            }
        }
        camera_file.close();
    }

    if (offscreen)
    {
        // glBindFramebuffer(GL_FRAMEBUFFER, msaa_rbo);
        // glViewport(0, 0, window_width, window_height); // 确保视口匹配 FBO 尺寸
        std::filesystem::create_directories("./offline_rendering");
    }

    for (int i = 0; i < frames; ++i)
    {
        printf("Render Frame %d -- Start\n", i);
        std::cerr << "Render Frame " << i << " -- Start" << std::endl;
        scene->wait_until_next_frame(i);
        if (CAMERA_ANIMATION)
        {
            if (20 * i >= camera_transforms.size())
                break;
            update_camera(camera_transforms[20 * i]);
        }
        else
        {
            update_camera(i);
        }
        render_frame(i);
        glfwPollEvents();
        printf("Render Frame %d -- End\n", i);
        std::cerr << "Render Frame " << i << " -- End" << std::endl;
    }
}

void RenderManager::update_camera()
{
    auto camera_pos = camera.get_pos();
    camera.set_position(camera_pos - glm::vec3(0, 0, 2.0));
    camera.compute_matrices_from_inputs(window, dirty);
}

void RenderManager::update_camera(glm::mat4 transform)
{
    // 从变换矩阵中提取旋转部分
    glm::mat3 rotation_matrix = glm::mat3(transform); // 提取 3x3 的旋转矩阵

    std::cout << "camera_matrix: " << glm::to_string(transform) << std::endl;

    // 从变换矩阵中提取平移部分
    glm::vec3 translation = glm::vec3(transform[0][3], transform[1][3], transform[2][3]);

    // 设置相机的新位置
    camera.set_position(translation);

    // 设置相机的旋转方向（可以通过相机的方向向量来控制相机朝向）
    camera.set_direction(rotation_matrix);
}

void RenderManager::update_camera(int current_frame)
{
    // 起始点和终点
    static glm::vec3 start_point = glm::vec3(0.0f, -30.0f, 180.0f);
    static glm::vec3 end_point = glm::vec3(-64.35f, -78.99f, -69.98f);

    // 插值因子
    float t = static_cast<float>(current_frame) / frames;

    // 使用线性插值来更新相机位置
    glm::vec3 interpolated_pos = glm::mix(start_point, end_point, t);

    camera.set_position(interpolated_pos);

    // 手动设置相机的方向向量
    glm::vec3 new_direction = glm::vec3(0.96f, 0.02f, -0.27f);
    camera.set_direction(new_direction);

    camera.set_fov(103.26 / 180.0f * glm::pi<float>());
}

void RenderManager::render_frame(int frame_number)
{
    // 当采样达到一定数量时将渲染结果提取出来
    scene->setDirty(true);
    if (offscreen)
    {
        // 循环进行渲染
        while (!scene->render_scene(camera))
        {
            // 当采样数达到一定的数量时生成一帧画面
            printf("SampleNumber: %d\n", scene->getSampleNum());
        }
    }
}