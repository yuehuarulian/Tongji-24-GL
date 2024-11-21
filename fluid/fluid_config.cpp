#include <iostream>  // 引入标准输入输出库
#include <fstream>   // 引入文件流库
#include <cmath>
#include <nlohmann/json.hpp> // 引入配置文件库

#include "fluid/fluid_config.h"  // 测试流体配置头文件

// 定义常量
constexpr double PI = 3.141592653589793;
using namespace fluid;

// 构建函数：初始化参数
FluidConfig::FluidConfig(fluid::simulation& simulation, const double scale, const std::function<bool(const vec3d&)>& basin, const std::function<bool(const vec3s&)>& buoy, const std::function<bool(const vec3s&)>& bathtub, const std::string& config_path)
    : _sim(simulation), _scale(scale),
	_basin(basin ? basin : [](const vec3d&) { return false; }), 
	_buoy(buoy ? buoy : [](const vec3s&) { return false; }),
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
	// 加载配置文件
	nlohmann::json config;
	std::ifstream config_file(_cfgfile);
	if (!config_file.is_open()) {
		throw std::runtime_error("Failed to open configuration file: " + _cfgfile);
	}
	config_file >> config;
	std::cout << "Init fluid configer by cfg_file: " << _cfgfile << std::endl;

	// 从配置文件读取参数
	_water_level = config.value("water_level", 0.35);
	_wave_amplitude = config.value("wave_amplitude", 3);
	
	// 初始化波纹
	initialize_waves();
}

// 重置流体环境
void FluidConfig::clear() {
    _sim.particles().clear();  // 清空粒子
	// 重置固体单元
	_sim.grid().grid().for_each(
		[](vec3s, fluid::mac_grid::cell& cell) {
			cell.cell_type = fluid::mac_grid::cell::type::air;  // 将所有单元设为空气
		}
	);
	// 设置外部边界为固体
	std::cout << "Seting solid cells..." << std::endl;
	std::cout << "  Grid size: (" << grid_size.x << ", " << grid_size.y << ", " << grid_size.z << ")" << std::endl;
	unsigned int total_cells = static_cast<unsigned int>(grid_size.x * grid_size.y * grid_size.z);
	unsigned int solid_cells = 0;
	// 初始化最小值为一个很大的数，最大值为一个很小的数
	_sim.grid().grid().for_each(
		[&](vec3s cell, fluid::mac_grid::cell& c) {
			if (_bathtub(cell)) {
				c.cell_type = fluid::mac_grid::cell::type::solid;
				++solid_cells;
				//std::cout << "solid cell " << solid_cells << " :(" << cell.x << ' ' << cell.y << ' ' << cell.z << ')' << std::endl
			}
		});
	std::cout << "  solid cells: " << solid_cells << " in " << total_cells << "." << std::endl;
	// 重置流体源和流体井
	_sim.sources.clear();
	_sim.drains.clear();
}

// 填充内部区域
void FluidConfig::fill_basin() {
	_sim.seed_area(grid_offset, vec3d(grid_size),
		[&](vec3d pos) {
			return
				pos.x <= grid_offset.x + double(grid_size.x) * _water_level + _wave_amplitude * _scale * simulate_pond_wave(pos.y, pos.z, 0.0) &&
				_basin(pos);
		},
		vec3d(0.0, 0.0, 0.0)
	);
	// 删除粒子
	/*_sim.remove_particles([&](const vec3d& pos) {
		return (pos.y - grid_center.y) * (pos.y - grid_center.y) + (pos.z - grid_center.z) * (pos.z - grid_center.z) <= grid_size.z * grid_size.z / 16;
		}, 1.0);*/
}

// 创建流体源并设置速度和位置
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
					source->cells.emplace_back(x, y, z);  // 添加流体源的单元位置
			}
		}
	}
	source->velocity = vec3d(0.0, 200.0, 0.0);  // 设置源的速度
	source->coerce_velocity = true;  // 强制流体源的速度
	_sim.sources.emplace_back(std::move(source));  // 将源添加到模拟中
}

// 创建流体井
void FluidConfig::set_drains() {
	return;

	auto drain = std::make_unique<fluid::drain>();
	for (std::size_t y = 1; y < 5; ++y) {
		for (std::size_t x = grid_size.x / 2 - grid_size.x / 20; x < grid_size.x / 2 + grid_size.x / 20; ++x) {
			for (std::size_t z = grid_size.z / 2 - grid_size.x / 20; z < grid_size.z / 2 + grid_size.x / 20; ++z) {
				if (_buoy(vec3s(x, y, z)))
					drain->cells.emplace_back(x, y, z);  // 添加流体源的单元位置
			}
		}
	}
	drain->percentage = 0.5;
	_sim.drains.emplace_back(std::move(drain));  // 将井添加到模拟中
}

// 模拟波动函数
double FluidConfig::simulate_pond_wave(double x, double y, double time) {
	double height = 0.0;

	// 叠加每个波的效果
	for (const auto& wave : waves) {
		double wave_component = wave.amplitude *
			sin(2 * PI * wave.frequency * (x * wave.direction_x + y * wave.direction_y) - wave.phase * time);
		height += wave_component;
	}

	// 引入噪声模拟微小扰动
	double noise = random_double(-0.05, 0.05);

	// 应用阻尼系数
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

	for (int i = 0; i < 5; ++i) { // 生成 5 个波
		Wave wave = {
			amplitude_dist(generator), // 随机幅度
			frequency_dist(generator), // 随机频率
			phase_dist(generator),     // 随机相位
			direction_dist(generator), // 随机方向 (x)
			direction_dist(generator)  // 随机方向 (y)
		};
		waves.push_back(wave);

		// 打印波的参数
		std::cout << "  Wave " << i + 1 << ": "
			<< "Amplitude = " << wave.amplitude
			<< ", Frequency = " << wave.frequency
			<< ", Phase = " << wave.phase
			<< ", Direction = (" << wave.direction_x << ", " << wave.direction_y << ")\n";
	}
}