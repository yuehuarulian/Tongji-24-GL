#ifndef SCENE_HPP
#define SCENE_HPP
#include <vector>
#include <memory>
#include "renderable.hpp"
#include "room.hpp"
namespace GL_TASK
{
    class Scene
    {
    public:
        void addObject(std::shared_ptr<Renderable> object)
        {
            objects.push_back(object);
        }

        void render(const glm::mat4 &P, const glm::mat4 &V, const glm::vec3 &cameraPos, const glm::vec3 &lightPos)
        {
            for (const auto &obj : objects)
            {
                obj->draw(P, V, cameraPos, lightPos);
            }
        }

    private:
        std::vector<std::shared_ptr<Renderable>> objects;
    };
} // namespace GL_TASK

#endif // SCENE_HPP