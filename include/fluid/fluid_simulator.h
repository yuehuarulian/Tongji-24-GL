#pragma once

#include <mutex>
#include <atomic>
#include <vector>
#include <thread>
#include <string>
#include <fluid/mesher.h>  // 网格类型
#include <fluid/simulation.h>  // 流体模拟相关的头文件
#include <fluid/data_structures/grid.h>  // 网格数据结构
#include <fluid/data_structures/obstacle.h>  // 静态障碍物类

#include "fluid/load_model.h" // 模型刷入
#include "fluid/fluid_config.h"  // 测试流体配置头文件
#include "mesh.hpp" // mesh绑定

namespace fluid {
    class FluidSimulator {
    public:
        FluidSimulator(bool def);//默认初始化
        FluidSimulator(const std::string& config_path = "fluid_config.json");//配置文件初始化
        // 绑定mesh
        void BindMesh(Mesh* const pMesh);
        // 重置与暂停
        void reset();
        void pause();
        void advance();
        // 返回参数
        double get_scale() const;
        double get_time() const;
        vec3d get_grid_offset() const;
        vec3s get_grid_size() const;
        vec3d get_grid_center() const;
        double get_cell_size() const;
        vec3d get_min_corner() const;
        vec3d get_max_corner() const;
        // 返回数据
        std::vector<simulation::particle> get_particles();
        std::vector<double> get_pressure();
        mesher::mesh_t get_mesh_t();
        grid3<std::size_t> get_grid_occupation();
        grid3<vec3d> get_grid_velocities();
        mesher::mesh_t get_room_mesh_t() const;
        // 导出数据
        void save_mesh_to_obj(std::string const& path = "mesh.obj");
        void save_points_to_txt(const std::string& filepath = "points.txt");
        // 同步函数
        void wait_until_next_sim(int i = -1);
        void wait_until_next_frame(int i = -1);

    private:
        // 配置文件路径
        std::string _cfgfile;

        // 导入模型
        double _scale;
        LoadModel roomModel;

        // 模拟基本参数
        vec3d sim_grid_offset;  // 网格偏移量
        vec3s sim_grid_size;  // 网格大小
        vec3d sim_grid_center; // 网格中心
        double sim_cell_size;  // 网格单元大小
        simulation::method sim_method; // 模拟方法
        double sim_blending_factor;  // 混合因子
        vec3d sim_gravity;  // 重力向量
        double sim_dt; // 时间步长
        double sim_time; // 模拟已经进行的时间

        // 将模型刷入网格
        obstacle roomObstacle;

        // 模拟和网格生成线程
        bool SimFinSignal{ false };
        bool MeshFinSignal{ false };
        std::thread sim_thread;
        std::thread mesh_thread;

        // mesh绑定
        bool isMeshBound;
        Mesh *pMesh;

        // 粒子和网格数据计算参数
        std::mutex sim_particles_lock;  // 用于同步粒子数据的互斥锁
        std::vector<simulation::particle> sim_particles;  // 粒子数据
        std::mutex sim_mesh_lock;  // 互斥锁，用于保护网格数据
        mesher::mesh_t sim_mesh;  // 网格数据
        grid3<std::size_t> sim_grid_occupation;  // 网格占用情况
        grid3<vec3d> sim_grid_velocities;  // 网格速度数据
        bool sim_mesh_valid = false;  // 网格有效性标志
        semaphore sim_mesher_sema;  // 用于控制网格生成的信号量

        // 定义一些原子变量，用于控制模拟状态
        std::atomic_bool
            sim_paused = false,  // 控制模拟是否暂停
            sim_reset = true,   // 控制模拟是否重置
            sim_advance = false;  // 控制模拟是否单步前进

        // 是否启用流体设置默认值
        bool _default;

        void update_simulation(const simulation& sim, FluidConfig& fluid_cfg);
        void reset_simulation(FluidConfig& fluid_cfg);
        void simulation_thread();
        void mesher_thread();
        int updateBoundMesh();
    };
}