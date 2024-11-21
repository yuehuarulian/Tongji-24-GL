#include "butterfly.hpp"
#include "glm/gtx/transform.hpp"
#include "error.hpp"

GL_TASK::Butterfly::Butterfly(const std::string &model_path, std::shared_ptr<Shader> shader, bool gamma)
    : RenderableModel(model_path, std::move(shader), gamma), danceAnimation(model_path, &model), animator(&danceAnimation) {}

void GL_TASK::Butterfly::draw(const glm::mat4 &projection, const glm::mat4 &view, const glm::vec3 &camera_pos)
{
    // float currentFrame = glfwGetTime() + differ;
    // deltaTime = currentFrame - lastFrame;
    // lastFrame = currentFrame;
    animator.UpdateAnimation(1 / 30.);

    shader->use();
    shader->setMat4("projection", projection);
    shader->setMat4("view", view);
    shader->setMat4("model", model_matrix);
    shader->setVec3("camPos", camera_pos);

    glm::mat4 keyframeTransforms;
    keyframeTransforms = animator.m_KeyframeTransforms["R"];
    shader->setMat4("keyframeTransforms", keyframeTransforms);
    model.Draw(*shader);
    keyframeTransforms = animator.m_KeyframeTransforms["L"];
    shader->setMat4("keyframeTransforms", keyframeTransforms);
    model.Draw(*shader);
}