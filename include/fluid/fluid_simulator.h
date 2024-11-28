#pragma once

#include <mutex>
#include <atomic>
#include <vector>
#include <thread>
#include <string>
#include <fluid/mesher.h>  // ��������
#include <fluid/simulation.h>  // ����ģ����ص�ͷ�ļ�
#include <fluid/data_structures/grid.h>  // �������ݽṹ
#include <fluid/data_structures/obstacle.h>  // ��̬�ϰ�����

#include "fluid/load_model.h" // ģ��ˢ��
#include "fluid/fluid_config.h"  // ������������ͷ�ļ�
#include "mesh.hpp" // mesh��

namespace fluid {
    class FluidSimulator {
    public:
        FluidSimulator(bool def);//Ĭ�ϳ�ʼ��
        FluidSimulator(const std::string& config_path = "fluid_config.json");//�����ļ���ʼ��
        // ��mesh
        void BindMesh(Mesh* const pMesh);
        void BindMeshSignal(bool* const pMeshSig);
        void BindSimSignal(bool* const pSimSig);
        // ��������ͣ
        void reset();
        void pause();
        void advance();
        // ���ز���
        double get_scale() const;
        double get_time() const;
        vec3d get_grid_offset() const;
        vec3s get_grid_size() const;
        vec3d get_grid_center() const;
        double get_cell_size() const;
        vec3d get_min_corner() const;
        vec3d get_max_corner() const;
        // ��������
        std::vector<simulation::particle> get_particles();
        mesher::mesh_t get_mesh_t();
        grid3<std::size_t> get_grid_occupation();
        grid3<vec3d> get_grid_velocities();
        mesher::mesh_t get_room_mesh_t() const;
        // ��������
        void save_mesh_to_obj(std::string const& path = "mesh.obj");
        void save_points_to_txt(const std::string& filepath = "points.txt");
        // ͬ������
        void wait_until_next_sim(int i = -1);
        void wait_until_next_frame(int i = -1);

    private:
        // �����ļ�·��
        std::string _cfgfile;

        // ����ģ��
        double _scale;
        LoadModel roomModel;

        // ģ���������
        vec3d sim_grid_offset;  // ����ƫ����
        vec3s sim_grid_size;  // �����С
        vec3d sim_grid_center; // ��������
        double sim_cell_size;  // ����Ԫ��С
        simulation::method sim_method; // ģ�ⷽ��
        double sim_blending_factor;  // �������
        vec3d sim_gravity;  // ��������
        double sim_dt; // ʱ�䲽��
        double sim_time; // ģ���Ѿ����е�ʱ��

        // ��ģ��ˢ������
        obstacle roomObstacle;

        // ģ������������߳�
        bool SimFinSignal{ false };
        bool* pSimFinSignal{ nullptr };
        bool MeshFinSignal{ false };
        bool* pMeshFinSignal{ nullptr };
        std::thread sim_thread;
        std::thread mesh_thread;

        // mesh��
        bool isMeshBound;
        Mesh *pMesh;

        // ���Ӻ��������ݼ������
        std::mutex sim_particles_lock;  // ����ͬ���������ݵĻ�����
        std::vector<simulation::particle> sim_particles;  // ��������
        std::mutex sim_mesh_lock;  // �����������ڱ�����������
        mesher::mesh_t sim_mesh;  // ��������
        grid3<std::size_t> sim_grid_occupation;  // ����ռ�����
        grid3<vec3d> sim_grid_velocities;  // �����ٶ�����
        bool sim_mesh_valid = false;  // ������Ч�Ա�־
        semaphore sim_mesher_sema;  // ���ڿ����������ɵ��ź���
        semaphore sim_updater_sema;  // ���ڿ�����һ��ģ����ź���

        // ����һЩԭ�ӱ��������ڿ���ģ��״̬
        std::atomic_bool
            sim_paused = false,  // ����ģ���Ƿ���ͣ
            sim_reset = true,   // ����ģ���Ƿ�����
            sim_advance = false;  // ����ģ���Ƿ񵥲�ǰ��

        // �Ƿ�������������Ĭ��ֵ
        bool _default;

        void update_simulation(const simulation& sim, FluidConfig& fluid_cfg);
        void reset_simulation(FluidConfig& fluid_cfg);
        void simulation_thread();
        void mesher_thread();
        int updateBoundMesh();
    };
}