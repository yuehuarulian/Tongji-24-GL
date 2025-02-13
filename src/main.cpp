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
    // 创建窗口
    GLFWwindow *window = initialize_glfw_window();

    ShaderManager shader_manager;
    LightManager light_manager;

    GL_TASK::ClassicScene classic_scene(shader_manager, light_manager);

    // Camera camera(window, 75 * D2R, glm::vec3(0.0f, -140.0f, 60.0f), glm::pi<float>(), 0. * D2R, 30.0f, 1.0f);
    Camera camera(window, 75 * D2R, glm::vec3(0.0f, -205.0f, 80.0f), glm::pi<float>(), 0. * D2R, 30.0f, 1.0f);
    // xyz -y z -x
    Skybox skybox(faces, "source/shader/skybox.vs", "source/shader/skybox.fs");

    GUIManager gui_manager(window, camera, light_manager, shader_manager);

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDepthFunc(GL_LESS);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD); // 增加混合的控制

    glfwSwapInterval(1);                                                                            // 垂直同步，参数：在 glfwSwapBuffers 交换缓冲区之前要等待的最小屏幕更新数
    while (glfwWindowShouldClose(window) == 0 && glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS) // 窗口没有关闭，esc键没有按下
    {
        bool dirty = false;
        camera.compute_matrices_from_inputs(window, dirty); // 更新摄像机位置
        classic_scene.setDirty(dirty);
        // if (classic_scene.getFrameNum() % 3 == 0 && classic_scene.getSampleNum() == 1)
        // {
        //     classic_scene.update_models(); // 蝴蝶每三帧画面更新一次
        // }
        // classic_scene.update_scene();

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDisable(GL_DEPTH_TEST);
        classic_scene.render_scene(camera);
        classic_scene.present_scene();
        gui_manager.render();

        glfwSwapBuffers(window);
        glfwPollEvents();
        // printf("SampleNumber: %d - FrameNum: %d\n", classic_scene.getSampleNum(), classic_scene.getFrameNum());
    }
    glfwTerminate();
    return 0;
}