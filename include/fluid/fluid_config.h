#pragma once

#include <vector>
#include <functional>
#include <fluid/simulation.h>  // 流体模拟相关的头文件

namespace fluid {

    class FluidConfig {
    public:
        FluidConfig(simulation& simulation, const double scale,
            const std::function<bool(const vec3d&)>& basin = nullptr, // 标识流体内部区域
            const std::function<bool(const vec3s&)>& buoy = nullptr, // 标识流体源与井区域
            const std::function<bool(const vec3s&)>& bathtub = nullptr, // 标识流体边缘区域
            const std::string& config_path = "" // 默认无配置文件
        );
        void apply();
        void update(double t);

    private:
        simulation& _sim;
        const std::function<bool(const vec3d&)> _basin;
        const std::function<bool(const vec3s&)> _buoy;
        const std::function<bool(const vec3s&)> _bathtub;
        const double _scale;
        const vec3d grid_offset;
        const vec3s grid_size;
        const vec3d grid_center;

        // 配置文件
        std::string _cfgfile;
        double _base_height;

        // 波纹参数
        struct Wave {
            double amplitude;
            double frequency;
            double phase;
            double direction_x;
            double direction_y;
        };
        std::vector<Wave> waves;
        double damping; // 阻尼系数

        // 配置函数
        void init_cfg();
        void clear();
        void fill_basin();
        void set_sources();
        void set_drains();

        // 功能配置函数
        double simulate_pond_wave(double x, double y, double time);
        double random_double(double min, double max);
        void initialize_waves();
    };

}