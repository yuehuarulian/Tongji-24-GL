#include <iostream>

#define GLM_ENABLE_EXPERIMENTAL
#include "glad/glad.h"
#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "GLFW/glfw3.h"

#include "camera_control.hpp"
#include "classic_scene.hpp"
#include "gui_manager.hpp"
#include "config.hpp"
#include "skybox.hpp"
#include "point_cloud.hpp"

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

GLFWwindow *initialize_glfw_window()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // 启用多重采样抗锯齿
    glfwWindowHint(GLFW_SAMPLES, 4); // 4x MSAA

    GLFWwindow *window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "scene", NULL, NULL);
    glfwMakeContextCurrent(window);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

    return window;
}

int main()
{
    GLFWwindow *window = initialize_glfw_window();

    ShaderManager shader_manager;
    LightManager light_manager;
    GL_TASK::ClassicScene classic_scene(shader_manager, light_manager);

    Camera camera(window, 75 * D2R, glm::vec3(0.0f, -30.0f, 180.0f), glm::pi<float>(), 0.f, 30.0f, 1.0f);

    Skybox skybox(faces, "source/shader/skybox.vs", "source/shader/skybox.fs");

    GUIManager gui_manager(window, camera, light_manager, shader_manager);

    shader_manager.load_shader("cload", "source/shader/point_cloud.vs", "source/shader/point_cloud.fs");
    auto cload_shader = shader_manager.get_shader("cload");
    PointCloud point_cloud("source/model/point_cloud/Cloud_01.vdb");

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD); // 增加混合的控制

    glfwSwapInterval(1);                                                                            // 垂直同步，参数：在 glfwSwapBuffers 交换缓冲区之前要等待的最小屏幕更新数
    while (glfwWindowShouldClose(window) == 0 && glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS) // 窗口没有关闭，esc键没有按下
    {
        camera.compute_matrices_from_inputs(window);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        auto camera_pos = camera.get_pos();
        classic_scene.render(camera.projection, camera.view, camera_pos);

        skybox.render(camera.view, camera.projection);
        // 在渲染点云之前禁用深度写入
        glDepthMask(GL_FALSE); // 禁止深度写入
        point_cloud.render(camera.view, camera.projection, camera_pos, cload_shader, glm::vec3(0.9f, 0.9f, 0.9f), 0.9f);
        glDepthMask(GL_TRUE); // 重新启用深度写入

        gui_manager.render();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}