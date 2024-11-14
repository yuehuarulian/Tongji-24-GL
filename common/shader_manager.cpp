#include "shader_manager.hpp"
#include <iostream>

void ShaderManager::load_shader(const std::string &name, const std::string &vertex_path, const std::string &fragment_path)
{
    if (shaders.find(name) == shaders.end())
    {
        shaders[name] = std::make_shared<Shader>(vertex_path, fragment_path);
    }
}

std::shared_ptr<Shader> ShaderManager::get_shader(const std::string &name)
{
    if (shaders.find(name) != shaders.end())
    {
        return shaders[name];
    }
    else
    {
        std::cerr << "Shader not found: " << name << std::endl;
        return nullptr;
    }
}
