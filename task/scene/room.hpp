#ifndef ROOM_HPP
#define ROOM_HPP

#include "shader.hpp"
#include "model.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/glm.hpp"
namespace GL_TASK
{
    class Room
    {
    public:
        Room(const std::string &modelPath, const std::string &vertexShaderPath, const std::string &fragmentShaderPath);
        ~Room();

        void draw(const glm::mat4 &P, const glm::mat4 &V, const glm::vec3 camera_pos, const glm::vec3 light_pos);

        Model model;
        Shader shader;

    private:
    };
}
#endif // ROOM_HPP
