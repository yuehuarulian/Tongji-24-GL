#include <iostream>

#define GLM_ENABLE_EXPERIMENTAL
#include "glad/glad.h"
#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "GLFW/glfw3.h"

#include "camera_control.hpp"
#include "classic_scene.hpp"

const unsigned int WINDOW_WIDTH = 1080 * 2;
const unsigned int WINDOW_HEIGHT = 720 * 2;

glm::vec3 LightPosition_worldspace = glm::vec3(0.0f, 0.0f, 0.0f); // 光源位置

// 初始化窗口和 OpenGL 上下文
GLFWwindow *initialize_glfw_window()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "scene", NULL, NULL);
    glfwMakeContextCurrent(window);
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

    Camera camera(window, 45.0f, glm::vec3(0.0f, 0.0f, 20.0f), glm::pi<float>(), 0.f, 5.0f, 4.0f);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glfwSwapInterval(1);                                                                            // 垂直同步，参数：在 glfwSwapBuffers 交换缓冲区之前要等待的最小屏幕更新数
    while (glfwWindowShouldClose(window) == 0 && glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS) // 窗口没有关闭，esc键没有按下
    {
        camera.compute_matrices_from_inputs(window);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        auto camera_pos = camera.get_pos();
        classic_scene.render(camera.projection, camera.view, camera_pos);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}