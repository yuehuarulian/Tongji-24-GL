#ifndef FLUID_HPP
#define FLUID_HPP

#include "shader.hpp"
#include "renderable_model.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/glm.hpp"
#include "fluid/fluid_simulator.h"

namespace GL_TASK
{
    class Fluid
    {
    public:
        Fluid(const std::string &model_path); // 默认初始化后先暂停模拟

        bool add_model(bool &bvhDirty, std::vector<Mesh *> &meshes, std::vector<MeshInstance *> &meshInstances, std::vector<Texture *> &textures, std::vector<Material> &materials);

        void update();

        void set_model_matrix(const glm::mat4 &room_model);

        glm::mat4 get_model_matrix() const;
        
        void start(); // 继续模拟
        
        void pause(); // 暂停模拟
        
        void advance(); // 模拟一帧（需要先暂停）

        void wait_until_next_frame(int frame);

        fluid::FluidSimulator fluid_sim;
    private:
        const std::string modelfilePath;
        glm::mat4 model_matrix = glm::mat4(1.0f);
    };
}

#endif // ROOM_HPP
