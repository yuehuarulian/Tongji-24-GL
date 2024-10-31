#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <filesystem>

#define GLM_ENABLE_EXPERIMENTAL
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "glad/glad.h"
#include "stb_image_write.h"
#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "GLFW/glfw3.h"

#include "camera_control.hpp"
#include "room.hpp"

const unsigned int WINDOW_WIDTH = 1080 * 2;
const unsigned int WINDOW_HEIGHT = 720 * 2;

glm::vec3 LightPosition_worldspace = glm::vec3(0.0f, 0.0f, 0.0f);

// 初始化窗口和 OpenGL 上下文
GLFWwindow *initialize_glfw_window()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE); // 隐藏窗口

    GLFWwindow *window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Offscreen", NULL, NULL);
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

    return window;
}

// 初始化帧缓冲区
unsigned int initialize_framebuffer(unsigned int &textureColorbuffer, unsigned int &rbo)
{
    unsigned int fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // 颜色缓冲纹理
    glGenTextures(1, &textureColorbuffer);
    glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);

    // 深度和模板缓冲
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, WINDOW_WIDTH, WINDOW_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return fbo;
}

// 保存帧缓冲区到图片
void save_framebuffer_to_image(int width, int height, int frame_number)
{
    std::vector<unsigned char> pixels(width * height * 3);
    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());

    std::ostringstream oss;
    oss << "./offline_rendering/frame_" << std::setw(3) << std::setfill('0') << frame_number << ".png";
    std::string filename = oss.str();

    stbi_flip_vertically_on_write(true);
    stbi_write_png(filename.c_str(), width, height, 3, pixels.data(), width * 3);
}

// 更新相机位置
void update_camera(Camera &camera, GLFWwindow *window)
{
    camera.set_position(camera.get_pos() + camera.get_direction() * 0.1f);
    camera.computeMatricesFromInputs(window);
}

// 渲染帧
void render_frame(Camera &camera, GL_TASK::Room &room, unsigned int fbo, int frame_number)
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const glm::mat4 P = camera.ProjectionMatrix;
    const glm::mat4 V = camera.ViewMatrix;
    room.draw(P, V, camera.get_pos(), LightPosition_worldspace);

    save_framebuffer_to_image(WINDOW_WIDTH, WINDOW_HEIGHT, frame_number);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

int main()
{
    GLFWwindow *window = initialize_glfw_window();
    GL_TASK::Room room("source/model/room.obj", "source/shader/room.vert", "source/shader/room.frag");
    Camera camera(window, 45.0f, glm::vec3(0.0f, 0.0f, 20.0f), glm::pi<float>(), 0.f, 5.0f, 4.0f);

    unsigned int textureColorbuffer, rbo;
    unsigned int fbo = initialize_framebuffer(textureColorbuffer, rbo);

    glEnable(GL_DEPTH_TEST);
    std::filesystem::create_directories("./offline_rendering");

    const int frames = 120;
    for (int i = 0; i < frames; ++i)
    {
        update_camera(camera, window);
        render_frame(camera, room, fbo, i);
        glfwPollEvents();
    }

    glDeleteFramebuffers(1, &fbo);
    glDeleteTextures(1, &textureColorbuffer);
    glDeleteRenderbuffers(1, &rbo);

    glfwTerminate();
    return 0;
}
