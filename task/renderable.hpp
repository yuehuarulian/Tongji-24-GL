#ifndef RENDERABLE_HPP
#define RENDERABLE_HPP
#include <glm/glm.hpp>
namespace GL_TASK
{
    class Renderable
    {
    public:
        virtual ~Renderable() = default;

        virtual void draw(const glm::mat4 &P, const glm::mat4 &V, const glm::vec3 camera_pos, const glm::vec3 light_pos) = 0;
    };
}
#endif // RENDERABLE_HPP
