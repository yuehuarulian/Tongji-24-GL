#include <iostream>  // 引入标准输入输出库
#include <fstream>   // 引入文件流库
#include <nlohmann/json.hpp> // 引入配置文件库

#include <fluid/mesher.h>  // 网格生成器
#include <fluid/data_structures/point_cloud.h>  // 点云数据结构

#include "fluid/fluid_simulator.h"  // 测试流体模拟头文件

using namespace fluid;

// 构建函数：初始化参数
FluidSimulator::FluidSimulator(bool def) :
	_default(def),
	_cfgfile(""),
	_scale(0.1), // 模型缩放规模
	roomModel("../assets/overall.obj", _scale, vec3d(0.0, 0.0, 0.0)), // 导入房间模型
	roomObstacle(roomModel.get_mesh(), sim_cell_size, sim_grid_offset, sim_grid_size), //将房间刷进网格
	sim_grid_offset(roomModel.get_offset() - vec3d(1.0, 1.0, 1.0)),  // 网格偏移量
	sim_grid_size(roomModel.get_size() + vec3d(2.0, 2.0, 2.0)),  // 网格大小
	sim_grid_center(roomModel.get_offset() + roomModel.get_size() / 2), // 网格中心
	sim_cell_size(1.0), // 网格单元大小
	sim_method(fluid::simulation::method::apic), // 使用apic模拟
	sim_blending_factor(1.0),  // 混合因子
	sim_gravity(vec3d(-981.0, 0.0, 0.0)),  // 重力向量
	sim_dt(1 / 60.0), // 时间步长
	sim_time(0.0), // 从时刻0开始模拟
	// 绑定mesh
	isMeshBound(false),
	pMesh(nullptr)
{
	// 使用 lambda 表达式将成员函数绑定到当前实例
	sim_thread = std::thread([this]() { simulation_thread(); });
	mesh_thread = std::thread([this]() { mesher_thread(); });
	// 将线程设为分离模式
	sim_thread.detach();
	mesh_thread.detach();
};

FluidSimulator::FluidSimulator(const std::string& config_path) : 
	_default(false) {
	// 加载配置文件
	nlohmann::json config;
	std::ifstream config_file(config_path);
	if (!config_file.is_open()) {
		throw std::runtime_error("Failed to open configuration file: " + config_path);
	}
	config_file >> config;
	std::cout << "Init fluid simulator by cfg_file: " << config_path << std::endl;

	// 从配置文件读取参数
	std::string model_path = config["model_path"];
	_scale = config.value("scale", 1.0);
	vec3d model_offset = vec3d(
		config["model_offset"][0],
		config["model_offset"][1],
		config["model_offset"][2]
	);
	roomModel = LoadModel(model_path, _scale, model_offset); // 导入房间模型
	sim_grid_offset = roomModel.get_offset() - vec3d(1.0, 1.0, 1.0);  // 网格偏移量
	sim_grid_size = vec3s(roomModel.get_size() + vec3d(2.0, 2.0, 2.0));  // 网格大小
	sim_grid_center = roomModel.get_offset() + roomModel.get_size() / 2; // 网格中心
	sim_cell_size = config.value("sim_cell_size", 1.0);  // 网格单元大小
	roomObstacle = obstacle(roomModel.get_mesh(), sim_cell_size, sim_grid_offset, sim_grid_size); // 将房间模型刷入网格
	sim_method = (config["sim_method"] == "pic" ? fluid::simulation::method::pic :
				(config["sim_method"] == "flip" ? fluid::simulation::method::flip_blend :
				(config["sim_method"] == "apic" ? fluid::simulation::method::apic : 
					fluid::simulation::method::apic))); // 使用apic模拟
	sim_blending_factor = config.value("sim_blending_factor", 1.0);  // 混合因子
	sim_gravity = vec3d(
		config["sim_gravity"][0],
		config["sim_gravity"][1],
		config["sim_gravity"][2]
	);  // 重力向量
	sim_dt = config.value("sim_dt", 1.0 / 60.0); // 时间步长
	sim_time = config.value("sim_time", 0.0); // 从时刻0开始模拟

	// 绑定mesh
	isMeshBound = false;
	pMesh = nullptr;
	
	// 线程初始化
	sim_thread = std::thread([this]() { simulation_thread(); });
	mesh_thread = std::thread([this]() { mesher_thread(); });
	sim_thread.detach();
	mesh_thread.detach();
}

// 更新模拟状态的函数
void FluidSimulator::update_simulation(const fluid::simulation& sim, FluidConfig& fluid_cfg) {
	// 更新流体状态
	fluid_cfg.update(sim_time);
	// 收集新的粒子数据
	std::vector<fluid::simulation::particle> new_particles(sim.particles().begin(), sim.particles().end());

	double energy = 0.0;  // 用于计算总能量
	for (fluid::simulation::particle& p : new_particles) {
		energy += 0.5 * p.velocity.squared_length();  // 动能
		energy -= fluid::vec_ops::dot(sim.gravity, p.position);  // 重力势能
	}
	std::cout << "    total energy: " << energy << "\n";  // 输出总能量

	// 收集网格的占用情况
	fluid::grid3<std::size_t> grid(sim.grid().grid().get_size(), 0);  // 初始化网格
	for (const auto& particle : sim.particles()) {
		vec3s pos(fluid::vec3i((particle.position - sim.grid_offset) / sim.cell_size));  // 计算粒子所在的网格位置
		if (pos.x < grid.get_size().x && pos.y < grid.get_size().y && pos.z < grid.get_size().z) {
			++grid(pos);  // 更新网格的占用计数
		}
	}

	// 收集网格的速度数据
	fluid::grid3<vec3d> grid_vels(sim.grid().grid().get_size());  // 初始化速度网格
	for (std::size_t z = 0; z < grid_vels.get_size().z; ++z) {
		for (std::size_t y = 0; y < grid_vels.get_size().y; ++y) {
			for (std::size_t x = 0; x < grid_vels.get_size().x; ++x) {
				grid_vels(x, y, z) = sim.grid().grid()(x, y, z).velocities_posface;  // 获取每个网格单元的速度
			}
		}
	}

	// 使用互斥锁保护对共享数据的访问
	{
		std::lock_guard<std::mutex> guard(sim_particles_lock);  // 锁定粒子数据
		sim_particles = std::move(new_particles);  // 更新粒子数据
		sim_grid_occupation = std::move(grid);  // 更新网格占用情况
		sim_grid_velocities = std::move(grid_vels);  // 更新网格速度
		sim_mesh_valid = false;  // 标记网格为无效
		sim_mesher_sema.notify();  // 通知网格生成线程
		SimFinSignal = true;
	}
}

void FluidSimulator::reset_simulation(FluidConfig& fluid_cfg) {
	fluid_cfg.apply();
};

// 模拟线程，管理流体模拟的执行
void FluidSimulator::simulation_thread() {
	fluid::simulation sim;  // 创建模拟对象

	sim.resize(sim_grid_size);  // 设置模拟网格大小
	sim.grid_offset = sim_grid_offset;  // 设置网格偏移量
	sim.cell_size = sim_cell_size;  // 设置网格单元大小
	sim.simulation_method = sim_method;  // 设置模拟方法为 APIC（Affine Particle-In-Cell）
	sim.blending_factor = sim_blending_factor;  // 设置混合因子
	sim.gravity = sim_gravity;  // 设置重力向量
	sim.total_time = sim_time;  // 设置初始时间

	// 在每个时间步之前的回调函数，用于输出时间步信息
	sim.pre_time_step_callback = [](double dt) {
		std::cout << "  time step " << dt << "\n";
		};
	// 在压力求解之后的回调函数，用于输出求解信息
	sim.post_pressure_solve_callback = [&sim](
		double, std::vector<double>& pressure, double residual, std::size_t iters
		) {
			std::cout << "    iterations = " << iters << "\n";
			if (iters > 100) {
				std::cout << "*** WARNING: large number of iterations\n";  // 提示迭代次数过多
			}
			std::cout << "    residual = " << residual << "\n";  // 输出残差
			auto max_it = std::max_element(pressure.begin(), pressure.end());
			if (max_it != pressure.end()) {
				std::cout << "    max pressure = " << *max_it << "\n";  // 输出最大压力
			}
		};
	// 在网格到粒子传输后的回调函数，输出粒子最大速度
	sim.post_grid_to_particle_transfer_callback = [&sim](double) {
		double maxv = 0.0;
		for (const fluid::simulation::particle& p : sim.particles()) {
			maxv = std::max(maxv, p.velocity.squared_length());  // 获取最大速度
		}
		std::cout << "    max particle velocity = " << std::sqrt(maxv) << "\n";
		};

	// 配置流体状态
	FluidConfig sim_cfg(sim, _scale,
		// 标识流体内部区域
		[&](vec3d pos) {return roomObstacle.is_cell_inside(pos); },
		// 标识流体源与井区域
		[&](vec3s pos) {return roomObstacle.is_cell_inside(pos); },
		// 标识流体边缘区域
		[&](vec3s pos) {return roomObstacle.is_cell_on_surface(pos); },
		// 配置文件路径
		_cfgfile
	);

	// 模拟主循环
	while (true) {
		if (sim_reset) {  // 如果需要重置
			reset_simulation(sim_cfg);
			update_simulation(sim, sim_cfg);  // 更新模拟状态
			sim_reset = false;  // 重置完成
		}

		if (!sim_paused) {  // 如果未暂停
			std::cout << "update\n";
			sim.update(sim_dt);  // 进行一个时间步长的模拟更新
			update_simulation(sim, sim_cfg);  // 更新模拟状态
		}
		else if (sim_advance) {  // 如果设置为单步前进
			sim_advance = false;
			sim.time_step();  // 进行一个时间步长
			update_simulation(sim, sim_cfg);  // 更新模拟状态
		}
		sim_time = sim.total_time; // 每一时间步更新模拟总时间
		std::cout << "One sim thread end. [total time = " << sim_time << "s]" << std::endl;
	}
}

void FluidSimulator::mesher_thread() {
	while (true) {
		sim_mesher_sema.wait();  // 等待信号量，确认网格需要更新
		std::vector<vec3d> particles;
		{
			std::lock_guard<std::mutex> lock(sim_particles_lock);  // 加锁以访问粒子数据
			if (sim_mesh_valid) {
				continue;  // 如果网格数据有效，则跳过更新
			}
			for (const fluid::simulation::particle& p : sim_particles) {
				particles.emplace_back(p.position);  // 收集所有粒子的位置信息
			}
			sim_mesh_valid = true;  // 标记网格数据为有效
		}

		// 使用网格生成器生成新的网格
		fluid::mesher mesher;
		mesher.particle_extent = 2.0;  // 设置粒子影响范围 // TODO values close or smaller than 1 causes holes to appear in meshes
		mesher.cell_radius = 3;  // 设置网格单元半径
		mesher.grid_offset = sim_grid_offset;  // 设置网格偏移
		mesher.cell_size = 0.5;  // 设置网格单元大小
		mesher.resize(sim_grid_size * 2);  // 调整网格尺寸
		fluid::mesher::mesh_t mesh = mesher.generate_mesh(particles, 0.5);  // 生成网格
		mesh.generate_normals();  // 生成法线，用于光照渲染

		{
			std::lock_guard<std::mutex> lock(sim_mesh_lock);  // 加锁以保护网格数据
			sim_mesh = std::move(mesh);  // 更新网格数据
			// 更新绑定mesh类实体
			if (!updateBoundMesh()) {
				MeshFinSignal = true;
			}
			std::ofstream fout("fluid_mesh.obj");
			sim_mesh.save_obj(fout);  // 导出网格为 .obj 文件
		}
	}
}

int FluidSimulator::updateBoundMesh() {
	// 未绑定则跳过
	if (!isMeshBound || !pMesh) {
		std::cout << "FluidSimulator:: unbound mesh" << endl;
		return -1;
	}

	std::cout << "FluidSimulator:: update mesh." << endl;
	std::cout << "	Before: " << pMesh->vertices.size() << " vertices and " << pMesh->indices.size() << " indices." << endl;
	// 清空旧的顶点和索引数据
	pMesh->vertices.clear();
	pMesh->indices.clear();

	// 遍历 simMesh.positions，填充 Mesh 的顶点数据
	for (size_t i = 0; i < sim_mesh.positions.size(); ++i) {
		Vertex vertex;

		// 设置顶点位置
		vertex.Position = glm::vec3(
			static_cast<float>(sim_mesh.positions[i].x),
			static_cast<float>(sim_mesh.positions[i].y),
			static_cast<float>(sim_mesh.positions[i].z)
		);

		// 设置法线（如果存在）
		if (i < sim_mesh.normals.size()) {
			vertex.Normal = glm::vec3(
				static_cast<float>(sim_mesh.normals[i].x),
				static_cast<float>(sim_mesh.normals[i].y),
				static_cast<float>(sim_mesh.normals[i].z)
			);
		}
		else {
			vertex.Normal = glm::vec3(0.0f, 0.0f, 0.0f); // 默认法线
		}

		// 设置纹理坐标（如果存在）
		if (i < sim_mesh.uvs.size()) {
			vertex.TexCoords = glm::vec2(
				static_cast<float>(sim_mesh.uvs[i].x),
				static_cast<float>(sim_mesh.uvs[i].y)
			);
		}
		else {
			vertex.TexCoords = glm::vec2(0.0f, 0.0f); // 默认纹理坐标
		}

		// 默认切线和副切线
		vertex.Tangent = glm::vec3(0.0f, 0.0f, 0.0f);
		vertex.Bitangent = glm::vec3(0.0f, 0.0f, 0.0f);

		// 骨骼影响（不适用）
		std::fill(std::begin(vertex.m_BoneIDs), std::end(vertex.m_BoneIDs), 0);
		std::fill(std::begin(vertex.m_Weights), std::end(vertex.m_Weights), 0.0f);

		pMesh->vertices.push_back(vertex);
	}

	// 填充索引数据
	for (size_t i = 0; i < sim_mesh.indices.size(); ++i) {
		pMesh->indices.push_back(static_cast<unsigned int>(sim_mesh.indices[i]));
	}

	// 调用 setupMesh 更新 VAO/VBO/EBO
	pMesh->updateMesh();
	std::cout << "	After: " << pMesh->vertices.size() << " vertices and " << pMesh->indices.size() << " indices." << endl;
	std::cout << "FluidSimulator:: update mesh." << endl;
	return 0;
}

//=====================================================================================
void FluidSimulator::pause() { sim_paused = !sim_paused; std::cout << (sim_paused ?  "Pause fluid simulation!" : "Continue fluid simulation!") << std::endl;}
void FluidSimulator::reset() { sim_reset = true; std::cout << "Reset fluid simulation!" << std::endl;}
void FluidSimulator::advance() { sim_advance = true; std::cout << "Excute fluid simulation one step!" << std::endl;}

void FluidSimulator::BindMesh(Mesh* const pm) {
	if (pm) {
		isMeshBound = true;
		pMesh = pm;
		std::cout << "FluidSimulator:: Mesh Bound successfully!" << std::endl;
	}
	else
		std::cout << "FluidSimulator:: Fail to bind Mesh!" << std::endl;
	return;
}

double FluidSimulator::get_scale() const {
	return _scale;
}
double FluidSimulator::get_time() const {
	return sim_time;
}
vec3d FluidSimulator::get_grid_offset() const {
	return sim_grid_offset;
}
vec3s FluidSimulator::get_grid_size() const {
	return sim_grid_size;
}
vec3d FluidSimulator::get_grid_center() const {
	return sim_grid_center;
}
double FluidSimulator::get_cell_size() const {
	return sim_cell_size;
}
vec3d FluidSimulator::get_min_corner() const {
	return sim_grid_offset;
}
vec3d FluidSimulator::get_max_corner() const {
	return sim_grid_offset + vec3d(sim_grid_size) * sim_cell_size;
}

std::vector<simulation::particle> FluidSimulator::get_particles() {
	std::lock_guard<std::mutex> guard(sim_particles_lock);
	return sim_particles;
}
mesher::mesh_t FluidSimulator::get_mesh_t() {
	std::lock_guard<std::mutex> guard(sim_mesh_lock);
	return sim_mesh;
}
grid3<std::size_t> FluidSimulator::get_grid_occupation() {
	std::lock_guard<std::mutex> guard(sim_particles_lock);
	return sim_grid_occupation;
}
grid3<vec3d> FluidSimulator::get_grid_velocities() {
	std::lock_guard<std::mutex> guard(sim_particles_lock);
	return sim_grid_velocities;
}
mesher::mesh_t FluidSimulator::get_room_mesh_t() const {
	return roomModel.get_mesh();
}

void FluidSimulator::save_mesh_to_obj(const std::string& filepath) {
	std::lock_guard<std::mutex> guard(sim_mesh_lock);
	std::ofstream fout(filepath);
	sim_mesh.save_obj(fout);  // 导出网格为 .obj 文件
	std::cout << "Have exported mesh data to " << filepath << std::endl;
}

void FluidSimulator::save_points_to_txt(const std::string& filepath) {
	std::vector<vec3d> points;
	{
		std::lock_guard<std::mutex> guard(sim_particles_lock);
		for (const fluid::simulation::particle& p : sim_particles) {
			points.emplace_back(p.position);  // 获取粒子位置信息
		}
	}
	std::ofstream fout(filepath);
	fluid::point_cloud::save_to_naive(fout, points.begin(), points.end());  // 导出粒子位置
	std::cout << "Have exported points data to points.txt" << std::endl;
}

// 同步函数
void FluidSimulator::wait_until_next_sim(int i) {
	while (!SimFinSignal) {
		continue;
	}
	SimFinSignal = false;
	std::cout << "FluidSimulator:: ";
	if (i >= 0) std::cout << "the " << i << "th";
	else std::cout << "a new";
	std::cout << " sim time step come." << std::endl;
}
void FluidSimulator::wait_until_next_frame(int i) {
	while (!MeshFinSignal) {
		continue;
	}
	MeshFinSignal = false;
	std::cout << "FluidSimulator:: ";
	if (i >= 0) std::cout << "the " << i << "th";
	else std::cout << "a new";
	std::cout << " frame come." << std::endl;
}