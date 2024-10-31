#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <filesystem>

#include "glad/glad.h"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "GLFW/glfw3.h"
#include "shader.hpp"
#include "camera_control.hpp"
#include "model.hpp"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h" // 用于将渲染的帧保存为图片

const unsigned int WINDOW_WIDTH = 1080 * 2;
const unsigned int WINDOW_HEIGHT = 720 * 2;

glm::vec3 LightPosition_worldspace = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 LightColor = glm::vec3(1, 1, 1);
float LightPower = 0.7f;

float specularStrength = 0.4f;

void save_framebuffer_to_image(int width, int height, int frame_number)
{
    // 获取帧缓冲区数据
    std::vector<unsigned char> pixels(width * height * 3);
    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());

    // 生成文件名
    std::ostringstream oss;
    oss << "./offline_rendering/frame_" << std::setw(3) << std::setfill('0') << frame_number << ".png";
    std::string filename = oss.str();

    // 保存图片
    stbi_flip_vertically_on_write(true); // 翻转图片
    stbi_write_png(filename.c_str(), width, height, 3, pixels.data(), width * 3);
}

int main()
{
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE); // 不显示窗口

    GLFWwindow *window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Offscreen", NULL, NULL);
    glfwMakeContextCurrent(window);

    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

    Model model("./source/model/test.obj");
    Shader shader("source/shader/house.vert", "source/shader/house.frag");
    Camera camera(window, 45.0f, glm::vec3(0.0f, 0.0f, 20.0f), glm::pi<float>(), 0.f, 5.0f, 4.0f);

    unsigned int fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    unsigned int textureColorbuffer;
    glGenTextures(1, &textureColorbuffer);
    glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);

    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, WINDOW_WIDTH, WINDOW_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glEnable(GL_DEPTH_TEST);

    int frames = 120; // 4 秒，每秒 30 帧，共 60 帧
    std::filesystem::create_directories("./offline_rendering");
    for (int i = 0; i < frames; ++i)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        camera.set_position(camera.get_pos() + camera.get_direction() * 0.1f);

        camera.computeMatricesFromInputs(window);
        glm::mat4 P = camera.ProjectionMatrix;
        glm::mat4 V = camera.ViewMatrix;
        glm::mat4 M = glm::mat4(1.0f);

        shader.use();
        M = glm::translate(M, glm::vec3(0.f, 0.f, 0.f));
        M = glm::rotate(M, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        M = glm::scale(M, glm::vec3(1.f, 1.f, 1.f) * 5.0f);
        shader.setMat4("M", M);
        shader.setMat4("V", V);
        shader.setMat4("P", P);
        shader.setVec3("CameraPosition_worldspace", camera.get_pos());
        shader.setVec3("pointlight.position", LightPosition_worldspace);
        shader.setVec3("pointlight.ambient", 0.8f, 0.8f, 0.8f);
        shader.setVec3("pointlight.diffuse", 0.8f, 0.8f, 0.8f);
        shader.setVec3("pointlight.specular", 1.0f, 1.0f, 1.0f);
        shader.setFloat("pointlight.constant", 1.0f);
        shader.setFloat("pointlight.linear", 0.09);
        shader.setFloat("pointlight.quadratic", 0.032);

        model.Draw(shader);

        save_framebuffer_to_image(WINDOW_WIDTH, WINDOW_HEIGHT, i);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glfwPollEvents();
    }

    glDeleteFramebuffers(1, &fbo);
    glDeleteTextures(1, &textureColorbuffer);
    glDeleteRenderbuffers(1, &rbo);

    glfwTerminate();
    return 0;
}
