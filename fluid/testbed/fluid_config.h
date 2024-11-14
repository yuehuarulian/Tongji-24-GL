#pragma once

#include <fluid/simulation.h>  // 流体模拟相关的头文件
#include <iostream>

using fluid::vec2d;  // 使用 fluid 命名空间中的二维向量类型
using fluid::vec2s;  // 使用 fluid 命名空间中的二维整数向量类型
using fluid::vec3d;  // 使用 fluid 命名空间中的三维向量类型
using fluid::vec3s;  // 使用 fluid 命名空间中的三维整数向量类型

namespace fluid {

    class FluidConfig {
    public:
        FluidConfig(fluid::simulation& simulation, const vec3d& grid_offset, const vec3s& grid_size)
            : sim(simulation), sim_grid_offset(grid_offset), sim_grid_size(grid_size) {}
    private:
        fluid::simulation& sim;
        vec3d sim_grid_offset;
        vec3s sim_grid_size;

        vec3d sim_grid_center() const {
            return vec3d(sim_grid_size) / 2.0 + sim_grid_offset;
        }
    };

}