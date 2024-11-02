#ifndef RENDERABLE_MODEL_H
#define RENDERABLE_MODEL_H

#include "model.hpp"
#include "shader.hpp"
#include <memory>
#include <glm/glm.hpp>

class RenderableModel
{
public:
    RenderableModel(const std::string &modelPath, std::shared_ptr<Shader> shader, bool gamma = false)
        : model(modelPath, gamma), shader(std::move(shader)) {}

    virtual void draw(const glm::mat4 &projection, const glm::mat4 &view, const glm::vec3 &cameraPos) = 0;

protected:
    std::shared_ptr<Shader> shader;
    Model model;
};

#endif // RENDERABLE_MODEL_H
