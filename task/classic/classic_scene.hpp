#include <vector>
#include <string>
#include <glad/glad.h>
#include <iostream>

#include "stb_image.h"
#include "shader.hpp" // 包含Shader类，用于加载和使用着色器

class Skybox
{
public:
    Skybox(const std::vector<std::string> &faces, const std::string &vertex_shader, const std::string &fragment_shader);
    void render(const glm::mat4 &view, const glm::mat4 &projection);
    ~Skybox();

private:
    unsigned int skybox_VAO, skybox_VBO, cubemap_texture;
    Shader skybox_shader;

    unsigned int load_cubemap(const std::vector<std::string> &faces);
    void setup_skybox();
};