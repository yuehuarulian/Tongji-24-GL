#ifndef ROOM_HPP
#define ROOM_HPP

#include "shader.hpp"
#include "renderable_model.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/glm.hpp"

namespace GL_TASK
{
    class Room : public RenderableModel
    {
    public:
        Room(const std::string &model_path, std::shared_ptr<Shader> shader, bool gamma = false);

        void draw(const glm::mat4 &projection, const glm::mat4 &view, const glm::vec3 &camera_pos) override;

        void set_model_matrix(const glm::mat4 &model) { model_matrix = model; }

        void getBoundingBox(glm::vec3 &min, glm::vec3 &max);

    private:
        glm::mat4 model_matrix = glm::mat4(1.0f);
        glm::vec3 min_bound;
        glm::vec3 max_bound;

        void calculateBoundingBox();
    };
}

#endif // ROOM_HPP
