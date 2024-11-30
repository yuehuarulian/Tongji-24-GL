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

    private:
        void set_model_matrix() override;

        bool add_model(const std::string &model_path) override;
    };
}

#endif // ROOM_HPP
