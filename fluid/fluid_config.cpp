#include <iostream>  // �����׼���������
#include <fstream>   // �����ļ�����
#include <cmath>
#include <nlohmann/json.hpp> // ���������ļ���

#include "fluid/fluid_config.h"  // ������������ͷ�ļ�

// ���峣��
constexpr double PI = 3.141592653589793;
using namespace fluid;

// ������������ʼ������
FluidConfig::FluidConfig(fluid::simulation& simulation, const double scale, const std::function<bool(const vec3d&)>& basin, const std::function<bool(const vec3s&)>& buoy, const std::function<bool(const vec3d&)>& batht, const std::function<bool(const vec3s&)>& bathtub, const std::string& config_path)
    : _sim(simulation), _scale(scale),
	_basin(basin ? basin : [](const vec3d&) { return false; }), 
	_buoy(buoy ? buoy : [](const vec3s&) { return false; }),
	_batht(batht ? batht : [](const vec3d&) { return false; }),
	_bathtub(bathtub ? bathtub : [](const vec3s&) { return false; }),
    grid_offset(_sim.grid_offset),
	grid_size(_sim.grid().grid().get_size()),
	grid_center(vec3d(grid_size) / 2.0 + grid_offset),
	_cfgfile(config_path)
{
	init_cfg();
};

void FluidConfig::apply() {
	std::cout << "Initializing liquid state..." << std::endl;
    clear();
    std::cout << "Start filling interior area..." << std::endl;
    fill_basin();
    std::cout << "Start seting liquid sources..." << std::endl;
    set_sources();
    std::cout << "Start seting liquid drains..." << std::endl;
    set_drains();
	std::cout << "Reset the spatial hash to update particles..." << std::endl;
	_sim.reset_space_hash();
    std::cout << "Finish initializing liquid state." << std::endl;
    std::cout << std::endl;
}

void FluidConfig::update(double t) {}

//=========================================================================================================
void FluidConfig::init_cfg() {
	if ("" == _cfgfile) {
		_water_level = 0.35;
		_wave_amplitude = 3;
		return;
	}
	// ���������ļ�
	nlohmann::json config;
	std::ifstream config_file(_cfgfile);
	if (!config_file.is_open()) {
		throw std::runtime_error("Failed to open configuration file: " + _cfgfile);
	}
	config_file >> config;
	std::cout << "Init fluid configer by cfg_file: " << _cfgfile << std::endl;

	// �������ļ���ȡ����
	_water_level = config.value("water_level", 0.35);
	_wave_amplitude = config.value("wave_amplitude", 3);
	
	// ��ʼ������
	initialize_waves();
}

// �������廷��
void FluidConfig::clear() {
    _sim.particles().clear();  // �������
	// ���ù��嵥Ԫ
	_sim.grid().grid().for_each(
		[](vec3s, fluid::mac_grid::cell& cell) {
			cell.cell_type = fluid::mac_grid::cell::type::air;  // �����е�Ԫ��Ϊ����
		}
	);
	// �����ⲿ�߽�Ϊ����
	std::cout << "Seting solid cells..." << std::endl;
	std::cout << "  Grid size: (" << grid_size.x << ", " << grid_size.y << ", " << grid_size.z << ")" << std::endl;
	unsigned int total_cells = static_cast<unsigned int>(grid_size.x * grid_size.y * grid_size.z);
	unsigned int solid_cells = 0, inner_cells = 0;
	// ��ʼ����СֵΪһ���ܴ���������ֵΪһ����С����
	_sim.grid().grid().for_each(
		[&](vec3s cell, fluid::mac_grid::cell& c) {
			if (_bathtub(cell)) {
				c.cell_type = fluid::mac_grid::cell::type::solid;
				++solid_cells;
				//std::cout << "solid cell " << solid_cells << " :(" << cell.x << ' ' << cell.y << ' ' << cell.z << ')' << std::endl
			}
			if (_buoy(cell)) {
				++inner_cells;
				//std::cout << "solid cell " << solid_cells << " :(" << cell.x << ' ' << cell.y << ' ' << cell.z << ')' << std::endl
			}
		});
	std::cout << "  solid cells: " << solid_cells << " inner cells: " << inner_cells << " in " << total_cells << "." << std::endl;
	// ��������Դ�����微
	_sim.sources.clear();
	_sim.drains.clear();
}

// ����ڲ�����
void FluidConfig::fill_basin() {
	_sim.seed_area(grid_offset, vec3d(grid_size),
		[&](vec3d pos) {
			return
				_basin(pos) && // �ж������ڷ����ڲ�
				//!_batht(pos) && // �ж������ڷ����Ե
				// ȷ��ˮ��߶�
				pos.x <= grid_offset.x + double(grid_size.x) * _water_level + _wave_amplitude * _scale * simulate_pond_wave(pos.y, pos.z, 0.0);
		},
		vec3d(0.0, 0.0, 0.0)
	);
	// ɾ������
	/*_sim.remove_particles([&](const vec3d& pos) {
		return (pos.y - grid_center.y) * (pos.y - grid_center.y) + (pos.z - grid_center.z) * (pos.z - grid_center.z) <= grid_size.z * grid_size.z / 16;
		}, 1.0);*/
}

// ��������Դ�������ٶȺ�λ��
void FluidConfig::set_sources() {
	return;

	auto src1 = std::make_unique<fluid::source>();
	auto src2 = std::make_unique<fluid::source>();
	for (std::size_t x = 1; x < grid_size.x * _water_level / 2; ++x) {
		for (std::size_t y = grid_size.y / 2 - 5; y < grid_size.y / 2 + 5; ++y) {
			for (std::size_t dz = grid_size.z / 4; dz < grid_size.z / 2; ++dz) {
				std::size_t bz = grid_size.z / 2;
				if (_buoy(vec3s(x, y, (bz + dz))))
					src1->cells.emplace_back(x, y, (bz + dz));
				if (_buoy(vec3s(x, y, (bz - dz))))
					src2->cells.emplace_back(x, y, (bz - dz));
			}
		}
	}
	src1->seed = src2->seed = false;
	src1->coerce_velocity = src2->coerce_velocity = true;
	src1->velocity = vec3d(0.0, 100.0, 0.0);
	src2->velocity = vec3d(0.0, -100.0, 0.0);
	_sim.sources.emplace_back(std::move(src1));
	_sim.sources.emplace_back(std::move(src2));

	auto source = std::make_unique<fluid::source>();
	for (std::size_t y = 1; y < 5; ++y) {
		for (std::size_t x = grid_size.x / 2 - grid_size.x / 20; x < grid_size.x / 2 + grid_size.x / 20; ++x) {
			for (std::size_t z = grid_size.z / 2 - grid_size.x / 20; z < grid_size.z / 2 + grid_size.x / 20; ++z) {
				if (_buoy(vec3s(x, y, z)))
					source->cells.emplace_back(x, y, z);  // ��������Դ�ĵ�Ԫλ��
			}
		}
	}
	source->velocity = vec3d(0.0, 200.0, 0.0);  // ����Դ���ٶ�
	source->coerce_velocity = true;  // ǿ������Դ���ٶ�
	_sim.sources.emplace_back(std::move(source));  // ��Դ���ӵ�ģ����
}

// �������微
void FluidConfig::set_drains() {
	return;

	auto drain = std::make_unique<fluid::drain>();
	for (std::size_t y = 1; y < 5; ++y) {
		for (std::size_t x = grid_size.x / 2 - grid_size.x / 20; x < grid_size.x / 2 + grid_size.x / 20; ++x) {
			for (std::size_t z = grid_size.z / 2 - grid_size.x / 20; z < grid_size.z / 2 + grid_size.x / 20; ++z) {
				if (_buoy(vec3s(x, y, z)))
					drain->cells.emplace_back(x, y, z);  // ��������Դ�ĵ�Ԫλ��
			}
		}
	}
	drain->percentage = 0.5;
	_sim.drains.emplace_back(std::move(drain));  // �������ӵ�ģ����
}

// ģ�Ⲩ������
double FluidConfig::simulate_pond_wave(double x, double y, double time) {
	double height = 0.0;

	// ����ÿ������Ч��
	for (const auto& wave : waves) {
		double wave_component = wave.amplitude *
			sin(2 * PI * wave.frequency * (x * wave.direction_x + y * wave.direction_y) - wave.phase * time);
		height += wave_component;
	}

	// ��������ģ��΢С�Ŷ�
	double noise = random_double(-0.05, 0.05);

	// Ӧ������ϵ��
	height *= pow(damping, time);

	return height + noise;
}
double FluidConfig::random_double(double min, double max) {
	static std::default_random_engine generator(std::random_device{}());
	std::uniform_real_distribution<double> distribution(min, max);
	return distribution(generator);
}
void FluidConfig::initialize_waves() {
	std::default_random_engine generator(std::random_device{}());
	std::uniform_real_distribution<double> amplitude_dist(0.3, 0.7);
	std::uniform_real_distribution<double> frequency_dist(1.0, 3.0);
	std::uniform_real_distribution<double> phase_dist(0.0, 2 * PI);
	std::uniform_real_distribution<double> direction_dist(-1.0, 1.0);

	std::cout << "Generated wave parameters:\n";

	for (int i = 0; i < 5; ++i) { // ���� 5 ����
		Wave wave = {
			amplitude_dist(generator), // �������
			frequency_dist(generator), // ���Ƶ��
			phase_dist(generator),     // �����λ
			direction_dist(generator), // ������� (x)
			direction_dist(generator)  // ������� (y)
		};
		waves.push_back(wave);

		// ��ӡ���Ĳ���
		std::cout << "  Wave " << i + 1 << ": "
			<< "Amplitude = " << wave.amplitude
			<< ", Frequency = " << wave.frequency
			<< ", Phase = " << wave.phase
			<< ", Direction = (" << wave.direction_x << ", " << wave.direction_y << ")\n";
	}
}