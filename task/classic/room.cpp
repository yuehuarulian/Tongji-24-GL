#include "room.hpp"
#include "glm/gtx/transform.hpp"

GL_TASK::Room::Room(const std::string &model_path, std::shared_ptr<Shader> shader, bool gamma)
    : RenderableModel(model_path, std::move(shader), gamma) {}

void GL_TASK::Room::draw(const glm::mat4 &projection, const glm::mat4 &view, const glm::vec3 &camera_pos)
{
    glm::mat4 M = glm::mat4(1.0f);
    M = glm::translate(M, glm::vec3(0.f, 0.f, 0.f));
    M = glm::rotate(M, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    M = glm::scale(M, glm::vec3(1.f, 1.f, 1.f) * 5.0f);

    shader->use();
    shader->setMat4("M", M);
    shader->setMat4("V", view);
    shader->setMat4("P", projection);
    shader->setVec3("CameraPosition_worldspace", camera_pos);
    shader->setVec3("pointlight.position", glm::vec3(0.0f, 0.0f, 0.0f));
    shader->setVec3("pointlight.ambient", 0.8f, 0.8f, 0.8f);
    shader->setVec3("pointlight.diffuse", 0.8f, 0.8f, 0.8f);
    shader->setVec3("pointlight.specular", 1.0f, 1.0f, 1.0f);
    shader->setFloat("pointlight.constant", 1.0f);
    shader->setFloat("pointlight.linear", 0.09);
    shader->setFloat("pointlight.quadratic", 0.032);

    model.Draw(*shader);
}