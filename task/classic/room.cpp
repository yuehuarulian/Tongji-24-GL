#include "room.hpp"
#include "glm/gtx/transform.hpp"
#include "error.hpp"

GL_TASK::Room::Room(const std::string &model_path, std::shared_ptr<Shader> shader, bool gamma)
    : RenderableModel(model_path, std::move(shader), gamma) {}

void GL_TASK::Room::draw(const glm::mat4 &projection, const glm::mat4 &view, const glm::vec3 &camera_pos)
{
    checkGLError("room draw");
    glm::mat4 M = glm::mat4(1.0f);
    M = glm::translate(M, glm::vec3(0.f, 0.f, 0.f));
    M = glm::rotate(M, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    M = glm::rotate(M, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    M = glm::scale(M, glm::vec3(1.f, 1.f, 1.f) * 1.0f);

    shader->use();
    shader->setMat4("projection", projection);
    shader->setMat4("view", view);
    shader->setMat4("model", M);
    shader->setVec3("camPos", camera_pos);

    shader->setVec3("lightPositions[0]", glm::vec3(0.0f, 0.0f, 0.0f));    // 假设一个光源
    shader->setVec3("lightColors[0]", glm::vec3(300.0f, 300.0f, 300.0f)); // 假设光的颜色
    shader->setVec3("lightPositions[1]", glm::vec3(0.0f, 0.0f, 10.0f));   // 假设一个光源
    shader->setVec3("lightColors[1]", glm::vec3(300.0f, 300.0f, 300.0f)); // 假设光的颜色

    // shader->setVec3("pointlight.position", glm::vec3(0.0f, 0.0f, 0.0f));
    // shader->setVec3("pointlight.ambient", 0.8f, 0.8f, 0.8f);
    // shader->setVec3("pointlight.diffuse", 0.8f, 0.8f, 0.8f);
    // shader->setVec3("pointlight.specular", 1.0f, 1.0f, 1.0f);
    // shader->setFloat("pointlight.constant", 1.0f);
    // shader->setFloat("pointlight.linear", 0.09);
    // shader->setFloat("pointlight.quadratic", 0.032);

    model.Draw(*shader);
    // checkOpenGLError("After model draw");
}