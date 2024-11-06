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

// OpenGL 错误回调函数
void GLAPIENTRY openglErrorCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam)
{
    std::cerr << "OpenGL Error [" << id << "]: " << message << std::endl;
    std::cerr << " - Source: " << source << std::endl;
    std::cerr << " - Type: " << type << std::endl;
    std::cerr << " - Severity: " << severity << std::endl;
}

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

    // // 检查 OpenGL 版本信息，确保上下文创建成功
    // const GLubyte *renderer = glGetString(GL_RENDERER);
    // const GLubyte *version = glGetString(GL_VERSION);
    // std::cout << "Renderer: " << renderer << std::endl;
    // std::cout << "OpenGL version supported " << version << std::endl;

    // // 启用 OpenGL 调试输出并设置错误回调（如果支持）
    // if (glfwExtensionSupported("GL_ARB_debug_output"))
    // {
    //     glEnable(GL_DEBUG_OUTPUT);
    //     glDebugMessageCallback(openglErrorCallback, nullptr);
    //     std::cout << "OpenGL Debug Output enabled." << std::endl;
    // }
    // else
    // {
    //     std::cout << "GL_ARB_debug_output not supported." << std::endl;
    // }

    return window;
}

int main()
{
    GLFWwindow *window = initialize_glfw_window();

    ShaderManager shader_manager;
    LightManager light_manager;
    GL_TASK::ClassicScene classic_scene(shader_manager, light_manager);

    Camera camera(window, 45.0f, glm::vec3(0.0f, 0.0f, 40.0f), glm::pi<float>(), 0.f, 20.0f, 10.0f);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glfwSwapInterval(1);                                                                            // 垂直同步，参数：在 glfwSwapBuffers 交换缓冲区之前要等待的最小屏幕更新数
    while (glfwWindowShouldClose(window) == 0 && glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS) // 窗口没有关闭，esc键没有按下
    {
        camera.compute_matrices_from_inputs(window);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        auto camera_pos = camera.get_pos();
        classic_scene.render(camera.projection, camera.view, camera_pos);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}