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

    camera = Camera(window, 75 * D2R, glm::vec3(0.0f, -120.0f, 80.0f), glm::pi<float>(), 15. * D2R, 30.0f, 1.0f);
    scene = std::make_unique<GL_TASK::ClassicScene>(shader_manager, light_manager); // TODO
    skybox = std::make_unique<Skybox>(faces, "source/shader/skybox.vs", "source/shader/skybox.fs");
    glEnable(GL_DEPTH_TEST);
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

// void RenderManager::initialize_framebuffer()
// {
//     // 创建多重采样帧缓冲区
//     glGenFramebuffers(1, &msaa_fbo);
//     glBindFramebuffer(GL_FRAMEBUFFER, msaa_fbo);
//     // 创建多重采样纹理附件
//     glGenTextures(1, &msaa_texture);
//     glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msaa_texture);
//     glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGB, window_width, window_height, GL_TRUE);
//     glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, msaa_texture, 0);
//     // 创建多重采样的渲染缓冲附件（深度和模板）
//     glGenRenderbuffers(1, &msaa_rbo);
//     glBindRenderbuffer(GL_RENDERBUFFER, msaa_rbo);
//     glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, window_width, window_height);
//     glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, msaa_rbo);
//     if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
//         std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
//     glBindFramebuffer(GL_FRAMEBUFFER, 0);
//     // 创建用于解析多重采样结果的普通帧缓冲区
//     glGenFramebuffers(1, &resolve_fbo);
//     glBindFramebuffer(GL_FRAMEBUFFER, resolve_fbo);
//     glGenTextures(1, &resolve_texture);
//     glBindTexture(GL_TEXTURE_2D, resolve_texture);
//     glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, window_width, window_height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//     glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, resolve_texture, 0);
//     if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
//         std::cout << "ERROR::FRAMEBUFFER:: Resolve Framebuffer is not complete!" << std::endl;
//     glBindFramebuffer(GL_FRAMEBUFFER, 0);
// }

void RenderManager::start_rendering(bool offscreen)
{
    this->offscreen = offscreen;
    initialize();

    if (offscreen)
    {
        // glBindFramebuffer(GL_FRAMEBUFFER, msaa_rbo);
        // glViewport(0, 0, window_width, window_height); // 确保视口匹配 FBO 尺寸
        std::filesystem::create_directories("./offline_rendering");
    }

    for (int i = 0; i < frames; ++i)
    {
        scene->wait_until_next_frame(i);
        update_camera();
        render_frame(i);
        glfwPollEvents();
    }
}

void RenderManager::update_camera()
{
    auto camera_pos = camera.get_pos();
    camera.set_position(camera_pos - glm::vec3(0, 0, 2.0));
    camera.compute_matrices_from_inputs(window, dirty);
    dirty = true;
}

void RenderManager::render_frame(int frame_number)
{
    // if (offscreen)
    //     glBindFramebuffer(GL_FRAMEBUFFER, msaa_fbo);

    // glClearColor(0.f, 0.f, 0.f, 1.0f);
    // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // skybox->render(camera.view, camera.projection);

    scene->setDirty(dirty);
    scene->update();
    scene->render(camera);
    scene->present(); // 渲染结果展示

    if (offscreen)
    {
        glm::vec3 *output_frame_ptr = scene->DenoiseProcess();

        std::vector<unsigned char> pixels(window_width * window_height * 3);

        std::cout << "output_frame_ptr size" << sizeof(output_frame_ptr) << std::endl;

        // Copy data from output_frame_ptr to pixels
        for (int i = 0; i < window_width * window_height; ++i)
        {
            glm::vec3 color = output_frame_ptr[i];
            pixels[i * 3 + 0] = static_cast<unsigned char>(glm::clamp(color.r * 255.0f, 0.0f, 255.0f)); // Red
            pixels[i * 3 + 1] = static_cast<unsigned char>(glm::clamp(color.g * 255.0f, 0.0f, 255.0f)); // Green
            pixels[i * 3 + 2] = static_cast<unsigned char>(glm::clamp(color.b * 255.0f, 0.0f, 255.0f)); // Blue
        }

        // Save the image to disk
        std::ostringstream oss;
        oss << "./offline_rendering/frame_" << std::setw(3) << std::setfill('0') << frame_number << ".png";
        std::cout << "frame_number: " << frame_number << std::endl;
        stbi_flip_vertically_on_write(true);
        stbi_write_png(oss.str().c_str(), window_width, window_height, 3, pixels.data(), window_width * 3);
    }
}