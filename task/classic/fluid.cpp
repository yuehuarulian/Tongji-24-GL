#include "fluid.hpp"
#include "glm/gtx/transform.hpp"

GL_TASK::Fluid::Fluid(const std::string &model_path, std::shared_ptr<Shader> shader, bool gamma)
    : RenderableModel(model_path, std::move(shader), gamma)
{
    fluid_sim.BindMesh(model.meshes[0]);
}

void GL_TASK::Fluid::draw(const glm::mat4 &projection, const glm::mat4 &view, const glm::vec3 &camera_pos)
{
    shader->use();
    shader->setMat4("projection", projection);
    shader->setMat4("view", view);
    shader->setMat4("model", model_matrix);
    shader->setVec3("camPos", camera_pos);

    // model.Draw(*shader);
}

void GL_TASK::Fluid::set_model_matrix(const glm::mat4 &model)
{
    model_matrix = glm::scale(model, glm::vec3(1.f, 1.f, 1.f) * (1.f / float(fluid_sim.get_scale()))); // Adjust scale
}

glm::mat4 GL_TASK::Fluid::get_model_matrix() const
{
    return model_matrix;
}

void GL_TASK::Fluid::wait_until_next_frame(int frame)
{
    fluid_sim.wait_until_next_frame(frame);
}