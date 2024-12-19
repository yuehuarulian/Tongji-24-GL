#include <iostream>

#define GLM_ENABLE_EXPERIMENTAL
#include "glad/glad.h"
#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "GLFW/glfw3.h"

#include "camera_control.hpp"
#include "fluid_scene.hpp"
#include "gui_manager.hpp"
#include "config.hpp"
#include "skybox.hpp"
#include "fluid/fluid_simulator.h"

void printOpenGLInfo()
{
    const GLubyte *version = glGetString(GL_VERSION);
    const GLubyte *glslVersion = glGetString(GL_SHADING_LANGUAGE_VERSION);

    std::cerr << "OpenGL Version: " << version << std::endl;
    std::cerr << "GLSL Version: " << glslVersion << std::endl;
}

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
    printOpenGLInfo();

    fluid::FluidSimulator fluid_sim;

    ShaderManager shader_manager;
    LightManager light_manager;
    GL_TASK::FluidScene fluid_scene(shader_manager, light_manager);
    std::cout << "Mesh num:" << fluid_scene.models[1]->model.meshes.size() << std::endl;
    std::cout << "Mesh address:" << &(fluid_scene.models[1]->model.meshes[0]) << std::endl;
    fluid_sim.BindMesh(&(fluid_scene.models[1]->model.meshes[0])); // 绑定的1号模型的第0个mesh（水是一号）

    Camera camera(window, 75 * D2R, glm::vec3(0.0f, -30.0f, 180.0f), glm::pi<float>(), 0.f, 30.0f, 1.0f);

    Skybox skybox(faces, "source/shader/skybox.vs", "source/shader/skybox.fs");

    GUIManager gui_manager(window, camera, light_manager, shader_manager);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_MULTISAMPLE);

    glfwSwapInterval(1);                                                                            // 垂直同步，参数：在 glfwSwapBuffers 交换缓冲区之前要等待的最小屏幕更新数
    while (glfwWindowShouldClose(window) == 0 && glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS) // 窗口没有关闭，esc键没有按下
    {
        camera.compute_matrices_from_inputs(window);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        auto camera_pos = camera.get_pos();
        fluid_scene.render(camera.projection, camera.view, camera_pos);

        skybox.render(camera.view, camera.projection);
        gui_manager.render();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}