#ifndef ROOM_HPP
#define ROOM_HPP

#include "shader.hpp"
#include "model.hpp"
#include "renderable.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/glm.hpp"
namespace GL_TASK
{
    class Room : public Renderable
    {
    public:
        Room(const std::string &modelPath, const std::string &vertexShaderPath, const std::string &fragmentShaderPath);

        ~Room();

        void draw(const glm::mat4 &P, const glm::mat4 &V, const glm::vec3 camera_pos, const glm::vec3 light_pos);

    private:
        Model model;
        Shader shader;
    };

}
#endif // ROOM_HPP
