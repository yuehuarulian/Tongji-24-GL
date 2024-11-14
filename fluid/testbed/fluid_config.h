#pragma once

#include <fluid/simulation.h>  // ����ģ����ص�ͷ�ļ�
#include <iostream>

using fluid::vec2d;  // ʹ�� fluid �����ռ��еĶ�ά��������
using fluid::vec2s;  // ʹ�� fluid �����ռ��еĶ�ά������������
using fluid::vec3d;  // ʹ�� fluid �����ռ��е���ά��������
using fluid::vec3s;  // ʹ�� fluid �����ռ��е���ά������������

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