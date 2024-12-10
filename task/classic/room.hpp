#ifndef ROOM_HPP
#define ROOM_HPP

#include "renderable_model.hpp"
#define GLM_ENABLE_EXPERIMENTAL

namespace GL_TASK
{
    class Room : public RenderableModel
    {
    public:
        Room(const std::string &model_path, std::vector<Mesh *> &meshes, std::vector<MeshInstance *> &meshInstances, std::vector<Texture *> &textures, std::vector<Material> &materials);

        void update() override;

        glm::mat4 get_model_matrix() const;

        void getBoundingBox(glm::vec3 &min, glm::vec3 &max);

    private:
        void set_model_matrix() override;

        bool add_model(const std::string &model_path) override;

        glm::vec3 min_bound;
        glm::vec3 max_bound;
    };
}

#endif // ROOM_HPP
