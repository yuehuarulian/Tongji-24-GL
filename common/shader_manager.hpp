#ifndef SHADERMANAGER_HPP
#define SHADERMANAGER_HPP

#include "Shader.hpp"
#include <map>
#include <memory>
#include <string>

class ShaderManager
{
public:
    std::shared_ptr<Shader> get_shader(const std::string &name);
    void load_shader(const std::string &name, const std::string &vertex_path, const std::string &fragment_path);

private:
    std::map<std::string, std::shared_ptr<Shader>> shaders;
};

#endif
