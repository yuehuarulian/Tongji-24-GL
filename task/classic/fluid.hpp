#ifndef FLUID_HPP
#define FLUID_HPP

#include "shader.hpp"
#include "renderable_model.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/glm.hpp"
#include "fluid/fluid_simulator.h"

namespace GL_TASK
{
    class Fluid : public RenderableModel
    {
    public:
        Fluid(const std::string &model_path, std::shared_ptr<Shader> shader, bool gamma = false);

        void draw(const glm::mat4 &projection, const glm::mat4 &view, const glm::vec3 &camera_pos) override;

        void set_model_matrix(const glm::mat4 &model) { model_matrix = model; }
        fluid::FluidSimulator fluid_sim;

    private:
        glm::mat4 model_matrix = glm::mat4(1.0f);
    };
}

#endif // ROOM_HPP
