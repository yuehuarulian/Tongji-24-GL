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
#include "scene.hpp"

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
    // 创建窗口
    GLFWwindow *window = initialize_glfw_window();

    ShaderManager shader_manager;                                       // 着色器管理者
    LightManager light_manager;                                         // 光源管理者
    GL_TASK::ClassicScene classic_scene(shader_manager, light_manager); // 场景类 -- Important!! -- 配置场景

    Camera camera(window, 75 * D2R, glm::vec3(0.0f, 0.0f, -75.0f), glm::pi<float>(), 0.f, 30.0f, 1.0f); // 摄像机
    Skybox skybox(faces, "source/shader/skybox.vs", "source/shader/skybox.fs");                         // 天空盒
    GUIManager gui_manager(window, camera, light_manager);                                              // IMGUI

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glfwSwapInterval(1); // 垂直同步，参数：在 glfwSwapBuffers 交换缓冲区之前要等待的最小屏幕更新数
    while (glfwWindowShouldClose(window) == 0 && glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS)
    {
        bool dirty = false;
        camera.compute_matrices_from_inputs(window, dirty);
        classic_scene.setDirty(dirty);
        classic_scene.update();

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDisable(GL_DEPTH_TEST);

        classic_scene.updateCameraInfo(&camera); // 更新场景摄像机数据
        classic_scene.render();                  // 场景渲染
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
        classic_scene.present(); // 渲染结果展示
        gui_manager.render();    // imgUI渲染
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}