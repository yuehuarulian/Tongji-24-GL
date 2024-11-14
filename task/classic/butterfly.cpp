#include "butterfly.hpp"
#include "glm/gtx/transform.hpp"
#include "error.hpp"

GL_TASK::Butterfly::Butterfly(const std::string &model_path, std::shared_ptr<Shader> shader, bool gamma)
    : RenderableModel(model_path, std::move(shader), gamma) {}

void GL_TASK::Butterfly::draw(const glm::mat4 &projection, const glm::mat4 &view, const glm::vec3 &camera_pos)
{
    checkGLError("butterfly draw");
    
    shader->use();
    shader->setMat4("projection", projection);
    shader->setMat4("view", view);
    shader->setMat4("model", model_matrix);
    shader->setVec3("camPos", camera_pos);

    model.Draw(*shader);
    // checkOpenGLError("After model draw");
}