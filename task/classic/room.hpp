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
    };
}

#endif // ROOM_HPP
