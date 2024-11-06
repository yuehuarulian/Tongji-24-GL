#ifndef ENVIRONMENT_MAP_HPP
#define ENVIRONMENT_MAP_HPP
#include <string>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "shader.hpp"
#include "error.hpp"
#include "load_texture.hpp"
#include "draw_base_model.hpp"

// 用于捕获立方体贴图的投影矩阵和视图矩阵
const glm::mat4 capture_projection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
const glm::mat4 capture_views[] = {
    glm::lookAt(glm::vec3(0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
    glm::lookAt(glm::vec3(0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
    glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
    glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
    glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
    glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f))};

/**
 * @brief 创建一个环境立方体贴图 (cubemap)。
 *        该函数将 equirectangular 格式的 HDR 贴图转换为立方体贴图，
 *        用于物理渲染（PBR）中的环境映射。
 *
 * @param hdr_path HDR 环境贴图的文件路径。
 * @param shader 用于转换的着色器（通常为 equirectangular 到 cubemap 的转换着色器）。
 * @param capture_projection 用于捕获立方体贴图的投影矩阵。
 * @param capture_views 包含 6 个方向的视图矩阵数组，用于捕获立方体贴图的 6 个面。
 * @return 生成的环境立方体贴图的 OpenGL 纹理 ID。
 */
GLuint convert_equirectangular_to_cubemap(GLuint hdr_texture, const std::string &vertex_shader_path = "source/shader/cubemap.vs", const std::string &fragment_shader_path = "source/shader/equirectangular_to_cubemap.fs")
{
    // 初始化转换的着色器
    Shader equirectangular_to_cubemap_shader(vertex_shader_path.c_str(), fragment_shader_path.c_str());

    // 设置立方体贴图
    GLuint env_cubemap;
    glGenTextures(1, &env_cubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, env_cubemap);
    for (unsigned int i = 0; i < 6; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 512, 512, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // 创建和配置FBO和RBO
    GLuint capture_fbo, capture_rbo;
    glGenFramebuffers(1, &capture_fbo);
    glGenRenderbuffers(1, &capture_rbo);
    glBindFramebuffer(GL_FRAMEBUFFER, capture_fbo);
    glBindRenderbuffer(GL_RENDERBUFFER, capture_rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, capture_rbo);

    // 渲染立方体贴图
    equirectangular_to_cubemap_shader.use();
    equirectangular_to_cubemap_shader.setInt("equirectangularMap", 0);
    equirectangular_to_cubemap_shader.setMat4("projection", capture_projection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, hdr_texture);

    glViewport(0, 0, 512, 512);
    glBindFramebuffer(GL_FRAMEBUFFER, capture_fbo);
    for (unsigned int i = 0; i < 6; ++i)
    {
        equirectangular_to_cubemap_shader.setMat4("view", capture_views[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, env_cubemap, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        render_cube();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return env_cubemap;
}

/**
 * @brief 创建一个辐照度立方体贴图 (irradiance cubemap)。
 *        该函数通过卷积操作将环境立方体贴图转换为辐照度贴图，用于 PBR 中的间接漫反射光照。
 *
 * @param env_cubemap 已生成的环境立方体贴图的 OpenGL 纹理 ID。
 * @param irradiance_shader 用于计算辐照度的着色器。
 * @param capture_projection 用于捕获立方体贴图的投影矩阵。
 * @param capture_views 包含 6 个方向的视图矩阵数组，用于捕获立方体贴图的 6 个面。
 * @return 生成的辐照度立方体贴图的 OpenGL 纹理 ID。
 */
GLuint generate_irradiance_map(GLuint env_cubemap, const std::string &vertex_shader_path = "source/shader/cubemap.vs", const std::string &fragment_shader_path = "source/shader/irradiance_convolution.fs")
{
    Shader irradiance_shader(vertex_shader_path.c_str(), fragment_shader_path.c_str());

    // 创建辐照度立方体贴图
    GLuint irradiance_map;
    glGenTextures(1, &irradiance_map);
    glBindTexture(GL_TEXTURE_CUBE_MAP, irradiance_map);
    for (unsigned int i = 0; i < 6; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 32, 32, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    GLuint capture_fbo, capture_rbo;
    glGenFramebuffers(1, &capture_fbo);
    glGenRenderbuffers(1, &capture_rbo);
    glBindFramebuffer(GL_FRAMEBUFFER, capture_fbo);
    glBindRenderbuffer(GL_RENDERBUFFER, capture_rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, capture_rbo);

    irradiance_shader.use();
    irradiance_shader.setInt("environmentMap", 0);
    irradiance_shader.setMat4("projection", capture_projection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, env_cubemap);

    glViewport(0, 0, 32, 32);
    glBindFramebuffer(GL_FRAMEBUFFER, capture_fbo);
    for (unsigned int i = 0; i < 6; ++i)
    {
        irradiance_shader.setMat4("view", capture_views[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradiance_map, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        render_cube();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return irradiance_map;
}

#endif // ENVIRONMENT_MAP_H
