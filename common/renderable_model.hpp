#ifndef RENDERABLE_MODEL_H
#define RENDERABLE_MODEL_H

#include "model.hpp"
#include "shader.hpp"
#include <memory>
#include <glm/glm.hpp>

class RenderableModel
{
public:
    RenderableModel(const std::string &model_path, std::vector<Mesh *> &meshes, std::vector<MeshInstance *> &meshInstances, std::vector<Texture *> &textures, std::vector<Material> &materials)
        : meshes(meshes), meshInstances(meshInstances), textures(textures), materials(materials) {}

    virtual void update() = 0;

protected:
    virtual void set_model_matrix() = 0;

    virtual bool add_model(const std::string &model_path) = 0;

    glm::mat4 model_matrix;

    std::vector<Mesh *> &meshes;
    std::vector<MeshInstance *> &meshInstances;
    std::vector<Texture *> &textures;
    std::vector<Material> &materials;
};
#endif // RENDERABLE_MODEL_H
