#include <iostream>  // �����׼���������
#include <fstream>   // �����ļ�����

#include <fluid/mesher.h>  // ����������
#include <fluid/data_structures/point_cloud.h>  // �������ݽṹ

#include "fluid/fluid_simulator.h"  // ��������ģ��ͷ�ļ�

using fluid::vec2d;  // ʹ�� fluid �����ռ��еĶ�ά��������
using fluid::vec2s;  // ʹ�� fluid �����ռ��еĶ�ά������������
using fluid::vec3d;  // ʹ�� fluid �����ռ��е���ά��������
using fluid::vec3s;  // ʹ�� fluid �����ռ��е���ά������������

using namespace fluid;

// ������������ʼ������
FluidSimulator::FluidSimulator(double scale) :
	roomModel("source/model/room/overall.obj", scale, vec3d(0.0, 0.0, 0.0)), // ���뷿��ģ��
	roomObstacle(roomModel.get_mesh(), sim_cell_size, sim_grid_offset, sim_grid_size), //������ˢ������
	sim_grid_offset(roomModel.get_offset() - vec3d(1.0, 1.0, 1.0)),  // ����ƫ����
	sim_grid_size(roomModel.get_size() + vec3d(2.0, 2.0, 2.0)),  // �����С
	sim_grid_center(roomModel.get_offset() + roomModel.get_size() / 2), // ��������
	sim_cell_size(1.0), // ����Ԫ��С
	sim_method(fluid::simulation::method::apic), // ʹ��apicģ��
	sim_blending_factor(1.0),  // �������
	sim_gravity(vec3d(-981.0, 0.0, 0.0)),  // ��������
	sim_dt(1 / 60.0), // ʱ�䲽��
	// ��mesh
	isMeshBound(false),
	pMesh(nullptr)
{
	// ʹ�� lambda ����ʽ����Ա�����󶨵���ǰʵ��
	sim_thread = std::thread([this]() { simulation_thread(); });
	mesh_thread = std::thread([this]() { mesher_thread(); });
	// ���߳���Ϊ����ģʽ
	sim_thread.detach();
	mesh_thread.detach();
	std::cout << "fluid simulator start successfuly." << std::endl;
}

// ����ģ��״̬�ĺ���
void FluidSimulator::update_simulation(const fluid::simulation& sim) {
	// �ռ��µ���������
	std::vector<fluid::simulation::particle> new_particles(sim.particles().begin(), sim.particles().end());

	double energy = 0.0;  // ���ڼ���������
	for (fluid::simulation::particle& p : new_particles) {
		energy += 0.5 * p.velocity.squared_length();  // ����
		energy -= fluid::vec_ops::dot(sim.gravity, p.position);  // ��������
	}
	std::cout << "    total energy: " << energy << "\n";  // ���������

	// �ռ������ռ�����
	fluid::grid3<std::size_t> grid(sim.grid().grid().get_size(), 0);  // ��ʼ������
	for (const auto& particle : sim.particles()) {
		vec3s pos(fluid::vec3i((particle.position - sim.grid_offset) / sim.cell_size));  // �����������ڵ�����λ��
		if (pos.x < grid.get_size().x && pos.y < grid.get_size().y && pos.z < grid.get_size().z) {
			++grid(pos);  // ���������ռ�ü���
		}
	}

	// �ռ�������ٶ�����
	fluid::grid3<vec3d> grid_vels(sim.grid().grid().get_size());  // ��ʼ���ٶ�����
	for (std::size_t z = 0; z < grid_vels.get_size().z; ++z) {
		for (std::size_t y = 0; y < grid_vels.get_size().y; ++y) {
			for (std::size_t x = 0; x < grid_vels.get_size().x; ++x) {
				grid_vels(x, y, z) = sim.grid().grid()(x, y, z).velocities_posface;  // ��ȡÿ������Ԫ���ٶ�
			}
		}
	}

	// ʹ�û����������Թ������ݵķ���
	{
		std::lock_guard<std::mutex> guard(sim_particles_lock);  // ������������
		sim_particles = std::move(new_particles);  // ������������
		sim_grid_occupation = std::move(grid);  // ��������ռ�����
		sim_grid_velocities = std::move(grid_vels);  // ���������ٶ�
		sim_mesh_valid = false;  // �������Ϊ��Ч
		sim_mesher_sema.notify();  // ֪ͨ���������߳�
	}
}

void FluidSimulator::reset_simulation(fluid::simulation& sim) {
	sim.particles().clear();  // �������
	// ���ù��嵥Ԫ
	sim.grid().grid().for_each(
		[](vec3s, fluid::mac_grid::cell& cell) {
			cell.cell_type = fluid::mac_grid::cell::type::air;  // �����е�Ԫ��Ϊ����
		}
	);
	// �����ⲿ�߽�Ϊ����
	/*sim.grid().grid().for_each([&](vec3s cell, fluid::mac_grid::cell& c) {
		vec3d cell_position = sim.grid_offset + vec3d(cell) * sim.cell_size;
		if (is_outside_room(cell_position, room_mesh)) {
			c.cell_type = fluid::mac_grid::cell::type::solid;
		}
		});*/
		// ���������Ƕ���������ı����ͽ������
	std::cout << "Seting solid cells..." << std::endl;
	std::cout << "Grid size: (" << sim.grid().grid().get_size().x << ", " << sim.grid().grid().get_size().y << ", " << sim.grid().grid().get_size().z << ")" << std::endl;
	unsigned int total_cells = static_cast<unsigned int>(sim.grid().grid().get_size().x * sim.grid().grid().get_size().y * sim.grid().grid().get_size().z);
	unsigned int solid_cells = 0;
	// ��ʼ����СֵΪһ���ܴ���������ֵΪһ����С����
	vec3s min_coords(std::numeric_limits<std::size_t>::max(), std::numeric_limits<std::size_t>::max(), std::numeric_limits<std::size_t>::max());
	vec3s max_coords(0, 0, 0);
	sim.grid().grid().for_each(
		[&](vec3s cell, fluid::mac_grid::cell& c) {
			if (roomObstacle.is_cell_on_surface(cell)) {
				c.cell_type = fluid::mac_grid::cell::type::solid;
				++solid_cells;
				//std::cout << "solid cell " << solid_cells << " :(" << cell.x << ' ' << cell.y << ' ' << cell.z << ')' << std::endl
			}
		});
	std::cout << "solid cells: " << solid_cells << " in " << total_cells << "." << std::endl;
	std::cout << std::endl;
	// ��������Դ
	sim.sources.clear();

	// �������ñ�����ò�ͬ�ĳ�ʼ����״̬
	std::cout << "Initializing liquid state..." << std::endl;
	//����ڲ�����
	std::cout << "Start filling interior area..." << std::endl;
	sim.seed_area(sim_grid_offset, vec3d(sim_grid_size),
				[&](vec3d pos) {
					return
						pos.x < -double(sim_grid_size.x) / 4 + (pos.y + pos.z) / 5 &&
						(pos.y - sim_grid_center.y) * (pos.y - sim_grid_center.y) + (pos.z - sim_grid_center.z) * (pos.z - sim_grid_center.z) >= sim_grid_size.z * sim_grid_size.z / 16 &&
						roomObstacle.is_cell_inside(pos);
				},
				vec3d(0.0, 0.0, 0.0)
	);
	std::cout << "Starting seting liquid source..." << std::endl;
	// 创建一个流体源并设置速度和位置
	auto source = std::make_unique<fluid::source>();
	for (std::size_t y = 1; y < 5; ++y) {
				for (std::size_t x = sim_grid_size.x / 2 - sim_grid_size.x / 20; x < sim_grid_size.x / 2 + sim_grid_size.x / 20; ++x) {
					for (std::size_t z = sim_grid_size.z / 2 - sim_grid_size.x / 20; z < sim_grid_size.z / 2 + sim_grid_size.x / 20; ++z) {
						if (roomObstacle.is_cell_inside(vec3s(x, y, z)))
							source->cells.emplace_back(x, y, z);  // 添加流体源的单元位置
					}
				}
	}
	source->velocity = vec3d(0.0, 200.0, 0.0);  // 设置源的速度
	source->coerce_velocity = true;  // 强制流体源的速度
	sim.sources.emplace_back(std::move(source));  // 将源添加到模拟中
	std::cout << "Finish initializing liquid state." << std::endl;
	std::cout << std::endl;

	// ���ÿռ��ϣ�����ڸ�������λ�ú�״̬
	sim.reset_space_hash();
};

// ģ���̣߳���������ģ���ִ��
void FluidSimulator::simulation_thread() {
	fluid::simulation sim;  // ����ģ�����

	sim.resize(sim_grid_size);  // ����ģ�������С
	sim.grid_offset = sim_grid_offset;  // ��������ƫ����
	sim.cell_size = sim_cell_size;  // ��������Ԫ��С
	sim.simulation_method = sim_method;  // ����ģ�ⷽ��Ϊ APIC��Affine Particle-In-Cell��
	/*sim.blending_factor = 0.99;*/
	sim.blending_factor = sim_blending_factor;  // ���û������
	sim.gravity = sim_gravity;  // ������������

	// ��ÿ��ʱ�䲽֮ǰ�Ļص��������������ʱ�䲽��Ϣ
	sim.pre_time_step_callback = [](double dt) {
		std::cout << "  time step " << dt << "\n";
		};
	// ��ѹ�����֮��Ļص�������������������Ϣ
	sim.post_pressure_solve_callback = [&sim](
		double, std::vector<double>& pressure, double residual, std::size_t iters
		) {
			std::cout << "    iterations = " << iters << "\n";
			if (iters > 100) {
				std::cout << "*** WARNING: large number of iterations\n";  // ��ʾ������������
			}
			std::cout << "    residual = " << residual << "\n";  // ����в�
			auto max_it = std::max_element(pressure.begin(), pressure.end());
			if (max_it != pressure.end()) {
				std::cout << "    max pressure = " << *max_it << "\n";  // ������ѹ��
			}
		};
	// ���������Ӵ����Ļص������������������ٶ�
	sim.post_grid_to_particle_transfer_callback = [&sim](double) {
		double maxv = 0.0;
		for (const fluid::simulation::particle& p : sim.particles()) {
			maxv = std::max(maxv, p.velocity.squared_length());  // ��ȡ����ٶ�
		}
		std::cout << "    max particle velocity = " << std::sqrt(maxv) << "\n";
		};
	// ģ����ѭ��
	while (true) {
		if (sim_reset) {  // �����Ҫ����
			reset_simulation(sim);
			update_simulation(sim);  // ����ģ��״̬
			sim_reset = false;  // �������
		}

		if (!sim_paused) {  // ���δ��ͣ
			std::cout << "update\n";
			//sim.update(1.0 / 60.0);  // ����һ��ʱ�䲽����ģ�����
			sim.update(sim_dt);
			update_simulation(sim);  // ����ģ��״̬
		}
		else if (sim_advance) {  // �������Ϊ����ǰ��
			sim_advance = false;
			sim.time_step();  // ����һ��ʱ�䲽��
			update_simulation(sim);  // ����ģ��״̬
		}
		std::cout << "one sim thread end." << std::endl;
	}
}

void FluidSimulator::mesher_thread() {
	while (true) {
		sim_mesher_sema.wait();  // �ȴ��ź�����ȷ��������Ҫ����
		std::vector<vec3d> particles;
		{
			std::lock_guard<std::mutex> lock(sim_particles_lock);  // �����Է�����������
			if (sim_mesh_valid) {
				continue;  // �������������Ч������������
			}
			for (const fluid::simulation::particle& p : sim_particles) {
				particles.emplace_back(p.position);  // �ռ��������ӵ�λ����Ϣ
			}
			sim_mesh_valid = true;  // �����������Ϊ��Ч
		}

		// ʹ�����������������µ�����
		fluid::mesher mesher;
		mesher.particle_extent = 2.0;  // ��������Ӱ�췶Χ // TODO values close or smaller than 1 causes holes to appear in meshes
		mesher.cell_radius = 3;  // ��������Ԫ�뾶
		//mesher.grid_offset = vec3d(-1.0, -1.0, -1.0);  // ��������ƫ��
		mesher.grid_offset = sim_grid_offset;
		mesher.cell_size = 0.5;  // ��������Ԫ��С
		//mesher.resize(vec3s(104, 104, 104));  // ��������ߴ�
		mesher.resize(sim_grid_size * 2);
		fluid::mesher::mesh_t mesh = mesher.generate_mesh(particles, 0.5);  // ��������
		mesh.generate_normals();  // ���ɷ��ߣ����ڹ�����Ⱦ

		{
			std::lock_guard<std::mutex> lock(sim_mesh_lock);  // �����Ա�����������
			sim_mesh = std::move(mesh);  // ������������

			// ���°�mesh��ʵ��
			updateBoundMesh();
		}
	}
}

void FluidSimulator::updateBoundMesh() {
	// δ��������
	if (!isMeshBound || !pMesh) {
		std::cout << "FluidSimulator:: unbound mesh" << endl;
		return;
	}
	std::cout << "FluidSimulator:: update mesh." << endl;
	std::cout << "	Before: " << pMesh->vertices.size() << " vertices and " << pMesh->indices.size() << " indices." << endl;
	// ��վɵĶ������������
	pMesh->vertices.clear();
	pMesh->indices.clear();

	// ���� simMesh.positions����� Mesh �Ķ�������
	for (size_t i = 0; i < sim_mesh.positions.size(); ++i) {
		Vertex vertex;

		// ���ö���λ��
		vertex.Position = glm::vec3(
			static_cast<float>(sim_mesh.positions[i].x),
			static_cast<float>(sim_mesh.positions[i].y),
			static_cast<float>(sim_mesh.positions[i].z)
		);

		// ���÷��ߣ�������ڣ�
		if (i < sim_mesh.normals.size()) {
			vertex.Normal = glm::vec3(
				static_cast<float>(sim_mesh.normals[i].x),
				static_cast<float>(sim_mesh.normals[i].y),
				static_cast<float>(sim_mesh.normals[i].z)
			);
		}
		else {
			vertex.Normal = glm::vec3(0.0f, 0.0f, 0.0f); // Ĭ�Ϸ���
		}

		// �����������꣨������ڣ�
		if (i < sim_mesh.uvs.size()) {
			vertex.TexCoords = glm::vec2(
				static_cast<float>(sim_mesh.uvs[i].x),
				static_cast<float>(sim_mesh.uvs[i].y)
			);
		}
		else {
			vertex.TexCoords = glm::vec2(0.0f, 0.0f); // Ĭ����������
		}

		// Ĭ�����ߺ͸�����
		vertex.Tangent = glm::vec3(0.0f, 0.0f, 0.0f);
		vertex.Bitangent = glm::vec3(0.0f, 0.0f, 0.0f);

		// ����Ӱ�죨�����ã�
		std::fill(std::begin(vertex.m_BoneIDs), std::end(vertex.m_BoneIDs), 0);
		std::fill(std::begin(vertex.m_Weights), std::end(vertex.m_Weights), 0.0f);

		pMesh->vertices.push_back(vertex);
	}

	// �����������
	for (size_t i = 0; i < sim_mesh.indices.size(); ++i) {
		pMesh->indices.push_back(static_cast<unsigned int>(sim_mesh.indices[i]));
	}

	// ���� setupMesh ���� VAO/VBO/EBO
	pMesh->updateMesh();
	std::cout << "	After: " << pMesh->vertices.size() << " vertices and " << pMesh->indices.size() << " indices." << endl;
	std::cout << "FluidSimulator:: update mesh." << endl;
}

//=====================================================================================
void FluidSimulator::pause() { sim_paused = !sim_paused; std::cout << (sim_paused ?  "Pause fluid simulation!" : "Continue fluid simulation!") << std::endl;}
void FluidSimulator::reset() { sim_reset = true; std::cout << "Reset fluid simulation!" << std::endl;}
void FluidSimulator::advance() { sim_advance = true; std::cout << "Excute fluid simulation one step!" << std::endl;}

void FluidSimulator::BindMesh(Mesh *const pm) {
	if (pm) {
		isMeshBound = true;
		pMesh = pm;
		std::cout << "FluidSimulator:: Mesh Bound successfully!" << std::endl;
	}
	else
		std::cout << "FluidSimulator:: Fail to bind Mesh!" << std::endl;
	return;
}

vec3d FluidSimulator::get_sim_grid_offset() const {
	return sim_grid_offset;
}
vec3s FluidSimulator::get_sim_grid_size() const {
	return sim_grid_size;
}
vec3d FluidSimulator::get_sim_grid_center() const {
	return sim_grid_center;
}
double FluidSimulator::get_sim_cell_size() const {
	return sim_cell_size;
}
vec3d FluidSimulator::get_min_corner() const {
	return sim_grid_offset;
}
vec3d FluidSimulator::get_max_corner() const {
	return sim_grid_offset + vec3d(sim_grid_size) * sim_cell_size;
}

mesher::mesh_t FluidSimulator::get_room_mesh_t() const {
	return roomModel.get_mesh();
}
mesher::mesh_t FluidSimulator::get_sim_mesh_t() {
	std::lock_guard<std::mutex> guard(sim_mesh_lock);  // �����Ա�����������
	return sim_mesh;  // ������ǰ����������
}

void FluidSimulator::save_mesh_to_obj(const std::string& filepath) {
	std::lock_guard<std::mutex> guard(sim_mesh_lock);
	std::ofstream fout(filepath);
	sim_mesh.save_obj(fout);  // ��������Ϊ .obj �ļ�
	std::cout << "Have exported mesh data to " << filepath << std::endl;
}

void FluidSimulator::save_points_to_txt(const std::string& filepath) {
	std::vector<vec3d> points;
	{
		std::lock_guard<std::mutex> guard(sim_particles_lock);
		for (const fluid::simulation::particle& p : sim_particles) {
			points.emplace_back(p.position);  // ��ȡ����λ����Ϣ
		}
	}
	std::ofstream fout(filepath);
	fluid::point_cloud::save_to_naive(fout, points.begin(), points.end());  // ��������λ��
	std::cout << "Have exported points data to points.txt" << std::endl;
}