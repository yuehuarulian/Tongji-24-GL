#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <openvdb/openvdb.h>
#include <openvdb/io/File.h>
#include <vector>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "camera_control.hpp"
#include "skybox.hpp"
#include "shader.hpp"
#include "point_cloud.hpp"

// 初始化OpenGL窗口
GLFWwindow *initOpenGL()
{
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return nullptr;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(800, 600, "OpenVDB Cloud Visualization", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return nullptr;
    }
    glfwMakeContextCurrent(window);

    // 初始化GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return nullptr;
    }

    glEnable(GL_DEPTH_TEST);
    return window;
}

int main()
{
    // 初始化OpenGL
    GLFWwindow *window = initOpenGL();

    Camera camera(window, 45.0f, glm::vec3(0.0f, 0.0f, 3.0f));

    // 加载VDB数据
    std::cout << "Loading VDB data..." << std::endl;

    auto shader = std::make_shared<Shader>("source/shader/point_cloud.vs", "source/shader/point_cloud.fs");
    PointCloud pointCloud("F:/7309/CloudScapes-Blender/CloudScapes/volumes/Cumulonimbus_14.vdb", shader);
    const std::vector<std::string> faces{
        "source/skybox/sky/right.jpg",
        "source/skybox/sky/left.jpg",
        "source/skybox/sky/top.jpg",
        "source/skybox/sky/bottom.jpg",
        "source/skybox/sky/front.jpg",
        "source/skybox/sky/back.jpg"};
    Skybox skybox(faces, "source/shader/skybox.vs", "source/shader/skybox.fs");
    glEnable(GL_DEPTH_TEST);

    while (!glfwWindowShouldClose(window))
    {
        // 清空屏幕
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        camera.compute_matrices_from_inputs(window);
        glm::mat4 projection = camera.projection;
        glm::mat4 view = camera.view;
        glm::vec3 camera_pos = camera.get_pos();

        // 渲染点云
        pointCloud.draw(projection, view, camera_pos, glm::vec3(0.9f, 0.9f, 0.9f), 0.9f, false);

        skybox.render(view, projection);

        // 交换缓冲区
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
