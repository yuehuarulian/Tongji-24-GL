// Shader.hpp
#ifndef SHADER_HPP
#define SHADER_HPP

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include <glad/glad.h>
#include <glm/glm.hpp>

class Shader
{
public:
    unsigned int ID;

    // 构造函数，加载和编译着色器
    Shader(const char *vertexPath, const char *fragmentPath, const char *geometrypath = nullptr);
    Shader(const std::string &vertexPath, const std::string &fragmentPath, const std::string &geometry_path = "");

    // 使用着色器程序
    void use();

    // uniform 工具函数
    void setBool(const std::string &name, bool value) const;
    void setInt(const std::string &name, int value) const;
    void setFloat(const std::string &name, float value) const;
    void setVec3(const std::string &name, float x, float y, float z) const;
    void setVec3(const std::string &name, const glm::vec3 &value) const;
    void setMat3(const std::string &name, const glm::mat3 &mat) const;
    void setMat4(const std::string &name, const glm::mat4 &mat) const;

private:
    // 检查着色器编译/链接时的错误
    void checkCompileErrors(unsigned int shader, const std::string &type);
};

#endif // SHADER_HPP
