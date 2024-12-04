#pragma once

#include <vector>
#include <functional>
#include <fluid/simulation.h>  // ����ģ����ص�ͷ�ļ�

namespace fluid {

    class FluidConfig {
    public:
        FluidConfig(simulation& simulation, const double scale,
            const std::function<bool(const vec3d&)>& basin = nullptr, // ��ʶ�����ڲ�����
            const std::function<bool(const vec3s&)>& buoy = nullptr, // ��ʶ����Դ�뾮����
            const std::function<bool(const vec3d&)>& batht = nullptr, // ��ʶ�����Ե����
            const std::function<bool(const vec3s&)>& bathtub = nullptr, // ��ʶ�����Ե����
            const std::string& config_path = "" // Ĭ���������ļ�
        );
        void apply();
        void update(double t);

    private:
        simulation& _sim;
        const std::function<bool(const vec3d&)> _basin;
        const std::function<bool(const vec3s&)> _buoy;
        const std::function<bool(const vec3d&)> _batht;
        const std::function<bool(const vec3s&)> _bathtub;
        const double _scale;
        const vec3d grid_offset;
        const vec3s grid_size;
        const vec3d grid_center;

        // �����ļ�
        std::string _cfgfile;
        double _water_level;
        double _wave_amplitude;

        // ���Ʋ���
        struct Wave {
            double amplitude;
            double frequency;
            double phase;
            double direction_x;
            double direction_y;
        };
        std::vector<Wave> waves;
        double damping; // ����ϵ��

        // ���ú���
        void init_cfg();
        void clear();
        void fill_basin();
        void set_sources();
        void set_drains();

        // �������ú���
        double simulate_pond_wave(double x, double y, double time);
        double random_double(double min, double max);
        void initialize_waves();
    };

}