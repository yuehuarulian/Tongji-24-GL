#include "room.hpp"
#include "glm/gtx/transform.hpp"

namespace GL_TASK
{
    Room::Room(const std::string &modelPath, const std::string &vertexShaderPath, const std::string &fragmentShaderPath)
        : model(modelPath), shader(vertexShaderPath, fragmentShaderPath) {}

    Room::~Room() {}

    void Room::draw(const glm::mat4 &P, const glm::mat4 &V, const glm::vec3 camera_pos, const glm::vec3 light_pos)
    {
        glm::mat4 M = glm::mat4(1.0f);
        M = glm::translate(M, glm::vec3(0.f, 0.f, 0.f));
        M = glm::rotate(M, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        M = glm::scale(M, glm::vec3(1.f, 1.f, 1.f) * 5.0f);

        shader.use();
        shader.setMat4("M", M);
        shader.setMat4("V", V);
        shader.setMat4("P", P);
        shader.setVec3("CameraPosition_worldspace", camera_pos);
        shader.setVec3("pointlight.position", light_pos);
        shader.setVec3("pointlight.ambient", 0.8f, 0.8f, 0.8f);
        shader.setVec3("pointlight.diffuse", 0.8f, 0.8f, 0.8f);
        shader.setVec3("pointlight.specular", 1.0f, 1.0f, 1.0f);
        shader.setFloat("pointlight.constant", 1.0f);
        shader.setFloat("pointlight.linear", 0.09);
        shader.setFloat("pointlight.quadratic", 0.032);

        model.Draw(shader);
    }
}
