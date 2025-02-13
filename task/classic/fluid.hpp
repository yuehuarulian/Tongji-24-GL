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
        Fluid(std::vector<Mesh *> &meshes, std::vector<MeshInstance *> &meshInstances, std::vector<Texture *> &textures, std::vector<Material> &materials); // 默认初始化后先暂停模拟

        bool add_model(const std::string &modelfilePath);

        void BindDirty(bool *dirtyPtr) { fluid_sim.BindMeshSignal(dirtyPtr); }

        void update() override {};

        void set_model_matrix() override {};

        void set_model_matrix(const glm::mat4 &room_model);

        glm::mat4 get_model_matrix() const;

        double get_water_level(const glm::vec3 &roomMin, const glm::vec3 &roomMax) const; // 自动计算水面高度坐标

        void start(); // 继续模拟

        void pause(); // 暂停模拟

        void advance(); // 模拟一帧（需要先处于暂停状态）

        void wait_until_next_frame(int frame);

        fluid::FluidSimulator fluid_sim;

    private:
        const std::string modelfilePath;
        glm::mat4 model_matrix = glm::mat4(1.0f);
    };
}

#endif // ROOM_HPP
