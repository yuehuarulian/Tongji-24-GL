#include "render_manager.hpp"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include "config.hpp"
#include <iostream>
#include <vector>
#include <sstream>
#include <iomanip>
#include <filesystem>

RenderManager::RenderManager(int window_width, int window_height, int frames)
    : window_width(window_width), window_height(window_height), frames(frames), offscreen(false) {}

RenderManager::~RenderManager()
{
    if (offscreen)
    {
        glDeleteFramebuffers(1, &fbo);
        glDeleteTextures(1, &texture_color_buffer);
        glDeleteRenderbuffers(1, &rbo);
    }
    glfwTerminate();
}

void RenderManager::initialize()
{
    initialize_GLFW();

    camera = std::make_shared<Camera>(window, 90 * D2R, glm::vec3(0.0f, -30.0f, 180.0f), glm::pi<float>(), 0.f, 30.0f, 1.0f);
    scene = std::make_unique<GL_TASK::FluidScene>(shader_manager, light_manager);
    skybox = std::make_unique<Skybox>(faces, "source/shader/skybox.vs", "source/shader/skybox.fs");
    fluid_sim.BindMesh(&(scene->models[1]->model.meshes[0]));
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
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

    if (offscreen)
    {
        initialize_framebuffer();
    }
}

void RenderManager::initialize_framebuffer()
{
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // 纹理附件
    glGenTextures(1, &texture_color_buffer);
    glBindTexture(GL_TEXTURE_2D, texture_color_buffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, window_width, window_height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_color_buffer, 0);

    // 渲染缓冲附件: 深度+模板缓冲
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, window_width, window_height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RenderManager::start_rendering(bool offscreen)
{
    this->offscreen = offscreen;
    initialize();

    if (offscreen)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glViewport(0, 0, window_width, window_height); // 确保视口匹配 FBO 尺寸
        std::filesystem::create_directories("./offline_rendering");
    }

    for (int i = 0; i < frames; ++i)
    {
        fluid_sim.wait_until_next_frame();
        update_camera();
        render_frame(i);
        glfwPollEvents();
    }
}

void RenderManager::update_camera()
{
    auto camera_pos = camera->get_pos();
    camera->set_position(camera_pos - glm::vec3(0, 0, 0.4));
    camera->compute_matrices_from_inputs(window);
}

void RenderManager::render_frame(int frameNumber)
{
    if (offscreen)
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glClearColor(0.f, 0.f, 0.f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    auto camera_pos = camera->get_pos();
    scene->render(camera->projection, camera->view, camera_pos);
    skybox->render(camera->view, camera->projection);

    if (offscreen)
    {
        std::vector<unsigned char> pixels(window_width * window_height * 3);
        glReadPixels(0, 0, window_width, window_height, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());

        std::ostringstream oss;
        oss << "./offline_rendering/frame_" << std::setw(3) << std::setfill('0') << frameNumber << ".png";
        stbi_flip_vertically_on_write(true);
        stbi_write_png(oss.str().c_str(), window_width, window_height, 3, pixels.data(), window_width * 3);
    }

    if (offscreen)
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
