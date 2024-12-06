#include <iostream>			 // �����׼���������
#include <fstream>			 // �����ļ�����
#include <nlohmann/json.hpp> // ���������ļ���

#include <fluid/mesher.h>					   // ����������
#include <fluid/data_structures/point_cloud.h> // �������ݽṹ

#include "fluid/fluid_simulator.h" // ��������ģ��ͷ�ļ�

using namespace fluid;

// ������������ʼ������
FluidSimulator::FluidSimulator(bool def) : _default(def),
										   _cfgfile(""),
										   _scale(0.1),																		  // ģ�����Ź�ģ
										   roomModel("../assets/overall.obj", _scale, vec3d(0.0, 0.0, 0.0)),				  // ���뷿��ģ��
										   roomObstacle(roomModel.get_mesh(), sim_cell_size, sim_grid_offset, sim_grid_size), // ������ˢ������
										   sim_grid_offset(roomModel.get_offset() - vec3d(1.0, 1.0, 1.0)),					  // ����ƫ����
										   sim_grid_size(roomModel.get_size() + vec3d(2.0, 2.0, 2.0)),						  // �����С
										   sim_grid_center(roomModel.get_offset() + roomModel.get_size() / 2),				  // ��������
										   sim_cell_size(1.0),																  // ����Ԫ��С
										   sim_method(fluid::simulation::method::apic),										  // ʹ��apicģ��
										   sim_blending_factor(1.0),														  // �������
										   sim_gravity(vec3d(-981.0, 0.0, 0.0) * _scale),									  // ��������
										   sim_dt(1 / 60.0),																  // ʱ�䲽��
										   sim_time(0.0),																	  // ��ʱ��0��ʼģ��
										   // ��mesh
										   isMeshBound(false),
										   pMesh(nullptr)
{
	// ʹ�� lambda ����ʽ����Ա�����󶨵���ǰʵ��
	sim_thread = std::thread([this]()
							 { simulation_thread(); });
	mesh_thread = std::thread([this]()
							  { mesher_thread(); });
	// ���߳���Ϊ����ģʽ
	sim_thread.detach();
	mesh_thread.detach();
};

FluidSimulator::FluidSimulator(const std::string &config_path) : _default(false), _cfgfile(config_path)
{
	// ���������ļ�
	nlohmann::json config;
	std::ifstream config_file(config_path);
	if (!config_file.is_open())
	{
		throw std::runtime_error("FluidSimulator: Failed to open configuration file: " + config_path);
	}
	config_file >> config;
	std::cout << "Init fluid simulator by cfg_file: " << config_path << std::endl;

	// �������ļ���ȡ����
	std::string model_path = config["model_path"];
	_scale = config.value("scale", 1.0);
	vec3d model_offset = vec3d(
		config["model_offset"][0].get<double>() * _scale,
		config["model_offset"][1].get<double>() * _scale,
		config["model_offset"][2].get<double>() * _scale);
	roomModel = LoadModel(model_path, _scale, model_offset);																																																		 // ���뷿��ģ��
	sim_grid_offset = roomModel.get_offset() - vec3d(1.0, 1.0, 1.0);																																																 // ����ƫ����
	sim_grid_size = vec3s(roomModel.get_size() + vec3d(2.0, 2.0, 2.0));																																																 // �����С
	sim_grid_center = roomModel.get_offset() + roomModel.get_size() / 2;																																															 // ��������
	sim_cell_size = config.value("sim_cell_size", 1.0);																																																				 // ����Ԫ��С
	roomObstacle = obstacle(roomModel.get_mesh(), sim_cell_size, sim_grid_offset, sim_grid_size);																																									 // ������ģ��ˢ������
	sim_method = (config["sim_method"] == "pic" ? fluid::simulation::method::pic : (config["sim_method"] == "flip" ? fluid::simulation::method::flip_blend : (config["sim_method"] == "apic" ? fluid::simulation::method::apic : fluid::simulation::method::apic))); // ʹ��apicģ��
	sim_blending_factor = config.value("sim_blending_factor", 1.0);																																																	 // �������
	sim_gravity = vec3d(
		config["sim_gravity"][0].get<double>() * _scale,
		config["sim_gravity"][1].get<double>() * _scale,
		config["sim_gravity"][2].get<double>() * _scale); // ��������
	sim_dt = config.value("sim_dt", 1.0 / 60.0);		  // ʱ�䲽��
	sim_time = config.value("sim_time", 0.0);			  // ��ʱ��0��ʼģ��

	// ��mesh
	isMeshBound = false;
	pMesh = nullptr;

	// �̳߳�ʼ��
	sim_thread = std::thread([this]()
							 { simulation_thread(); });
	mesh_thread = std::thread([this]()
							  { mesher_thread(); });
	sim_thread.detach();
	mesh_thread.detach();
}

// ����ģ��״̬�ĺ���
void FluidSimulator::update_simulation(const fluid::simulation &sim, FluidConfig &fluid_cfg)
{
	// ��������״̬
	fluid_cfg.update(sim_time);
	// �ռ��µ���������
	std::vector<fluid::simulation::particle> new_particles(sim.particles().begin(), sim.particles().end());

	double energy = 0.0; // ���ڼ���������
	for (fluid::simulation::particle &p : new_particles)
	{
		energy += 0.5 * p.velocity.squared_length();			// ����
		energy -= fluid::vec_ops::dot(sim.gravity, p.position); // ��������
	}
	std::cout << "    total energy: " << energy << "\n"; // ���������

	// �ռ������ռ�����
	fluid::grid3<std::size_t> grid(sim.grid().grid().get_size(), 0); // ��ʼ������
	for (const auto &particle : sim.particles())
	{
		vec3s pos(fluid::vec3i((particle.position - sim.grid_offset) / sim.cell_size)); // �����������ڵ�����λ��
		if (pos.x < grid.get_size().x && pos.y < grid.get_size().y && pos.z < grid.get_size().z)
		{
			++grid(pos); // ���������ռ�ü���
		}
	}

	// �ռ�������ٶ�����
	fluid::grid3<vec3d> grid_vels(sim.grid().grid().get_size()); // ��ʼ���ٶ�����
	for (std::size_t z = 0; z < grid_vels.get_size().z; ++z)
	{
		for (std::size_t y = 0; y < grid_vels.get_size().y; ++y)
		{
			for (std::size_t x = 0; x < grid_vels.get_size().x; ++x)
			{
				grid_vels(x, y, z) = sim.grid().grid()(x, y, z).velocities_posface; // ��ȡÿ������Ԫ���ٶ�
			}
		}
	}

	// ʹ�û����������Թ������ݵķ���
	{
		std::lock_guard<std::mutex> guard(sim_particles_lock); // ������������
		sim_particles = std::move(new_particles);			   // ������������
		sim_grid_occupation = std::move(grid);				   // ��������ռ�����
		sim_grid_velocities = std::move(grid_vels);			   // ���������ٶ�
		sim_mesh_valid = false;								   // �������Ϊ��Ч
		// �����ź�
		sim_mesher_sema.notify(); // ֪ͨ���������߳�
		SimFinSignal = true;
		if (pSimFinSignal)
			*pSimFinSignal = true;
	}
}

void FluidSimulator::reset_simulation(FluidConfig &fluid_cfg)
{
	fluid_cfg.apply();
};

// ģ���̣߳���������ģ���ִ��
void FluidSimulator::simulation_thread()
{
	fluid::simulation sim; // ����ģ�����

	sim.resize(sim_grid_size);				   // ����ģ�������С
	sim.grid_offset = sim_grid_offset;		   // ��������ƫ����
	sim.cell_size = sim_cell_size;			   // ��������Ԫ��С
	sim.simulation_method = sim_method;		   // ����ģ�ⷽ��Ϊ APIC��Affine Particle-In-Cell��
	sim.blending_factor = sim_blending_factor; // ���û������
	sim.gravity = sim_gravity;				   // ������������
	sim.total_time = sim_time;				   // ���ó�ʼʱ��

	// ��ÿ��ʱ�䲽֮ǰ�Ļص��������������ʱ�䲽��Ϣ
	sim.pre_time_step_callback = [](double dt)
	{
		std::cout << "  time step " << dt << "\n";
	};
	// ��ѹ�����֮��Ļص�������������������Ϣ
	sim.post_pressure_solve_callback = [&sim](
										   double, std::vector<double> &pressure, double residual, std::size_t iters)
	{
		std::cout << "    iterations = " << iters << "\n";
		if (iters > 100)
		{
			std::cout << "*** WARNING: large number of iterations\n"; // ��ʾ������������
		}
		std::cout << "    residual = " << residual << "\n"; // ����в�
		auto max_it = std::max_element(pressure.begin(), pressure.end());
		if (max_it != pressure.end())
		{
			std::cout << "    max pressure = " << *max_it << "\n"; // ������ѹ��
		}
	};
	// ���������Ӵ����Ļص������������������ٶ�
	sim.post_grid_to_particle_transfer_callback = [&sim](double)
	{
		double maxv = 0.0;
		for (const fluid::simulation::particle &p : sim.particles())
		{
			maxv = std::max(maxv, p.velocity.squared_length()); // ��ȡ����ٶ�
		}
		std::cout << "    max particle velocity = " << std::sqrt(maxv) << "\n";
	};

	// ��������״̬
	FluidConfig sim_cfg(sim, _scale,
						// ��ʶ�����ڲ�����
						[&](vec3d pos)
						{ return roomObstacle.is_cell_inside(pos); },
						// ��ʶ����Դ�뾮����
						[&](vec3s pos)
						{ return roomObstacle.is_cell_inside(pos); },
						// ��ʶ�����Ե����
						[&](vec3d pos)
						{ return roomObstacle.is_cell_on_surface(pos); },
						// ��ʶ�����Ե����
						[&](vec3s pos)
						{ return roomObstacle.is_cell_on_surface(pos); },
						// �����ļ�·��
						_cfgfile);

	// ģ����ѭ��
	sim_updater_sema.notify(); // ����ģ������߳�
	while (true)
	{
		if (sim_reset)
		{ // �����Ҫ����
			reset_simulation(sim_cfg);
			// update_simulation(sim, sim_cfg);  // ����ģ��״̬
			sim_reset = false; // �������
		}

		if (!sim_paused)
		{ // ���δ��ͣ
			std::cout << "update\n";
			sim.update(sim_dt);				 // ����һ��ʱ�䲽����ģ�����
			sim_updater_sema.wait();		 // �ȴ��ź�����ȷ�������Ѿ�����
			update_simulation(sim, sim_cfg); // ����ģ��״̬
		}
		else if (sim_advance)
		{ // �������Ϊ����ǰ��
			std::cout << "update\n";
			sim.time_step(sim_dt);			 // ����һ��ʱ�䲽��
			update_simulation(sim, sim_cfg); // ����ģ��״̬
			sim_advance = false;
		}
		else
			continue;
		sim_time = sim.total_time; // ÿһʱ�䲽����ģ����ʱ��
		std::cout << "FluidSimulator: One sim thread end. [total time = " << sim_time << "s]" << std::endl;
	}
}

void FluidSimulator::mesher_thread()
{
	while (true)
	{
		sim_mesher_sema.wait(); // �ȴ��ź�����ȷ��������Ҫ����
		std::vector<vec3d> particles;
		{
			std::lock_guard<std::mutex> lock(sim_particles_lock); // �����Է�����������
			if (sim_mesh_valid)
			{
				continue; // �������������Ч������������
			}
			for (const fluid::simulation::particle &p : sim_particles)
			{
				particles.emplace_back(p.position); // �ռ��������ӵ�λ����Ϣ
			}
			sim_mesh_valid = true; // �����������Ϊ��Ч
		}

		// ʹ�����������������µ�����
		fluid::mesher mesher;
		mesher.particle_extent = 2.0;									   // ��������Ӱ�췶Χ // TODO values close or smaller than 1 causes holes to appear in meshes
		mesher.cell_radius = 3;											   // ��������Ԫ�뾶
		mesher.grid_offset = sim_grid_offset;							   // ��������ƫ��
		mesher.cell_size = 0.5;											   // ��������Ԫ��С
		mesher.resize(sim_grid_size * 2);								   // ��������ߴ�
		fluid::mesher::mesh_t mesh = mesher.generate_mesh(particles, 0.5); // ��������
		mesh.generate_normals();										   // ���ɷ��ߣ����ڹ�����Ⱦ

		{
			std::lock_guard<std::mutex> lock(sim_mesh_lock); // �����Ա�����������
			sim_mesh = std::move(mesh);						 // ������������
			// ���°�mesh��ʵ��
			updateBoundMesh();
			// std::ofstream fout("fluid_mesh.obj");
			// sim_mesh.save_obj(fout);  // ��������Ϊ .obj �ļ�
			//  �����ź�
			sim_updater_sema.notify(); // ֪ͨģ������߳�
			MeshFinSignal = true;
			if (pMeshFinSignal)
				*pMeshFinSignal = true;
		}

		std::cout << "FluidSimulator: One mesher thread end." << std::endl;
	}
}

int FluidSimulator::updateBoundMesh()
{
	// δ��������
	if (!isMeshBound || !pMesh)
	{
		std::cout << "FluidSimulator:: unbound mesh" << std::endl;
		return -1;
	}

	std::cout << "FluidSimulator:: update mesh start." << std::endl;
	std::cout << "	Before: " << pMesh->vertices.size() << " vertices and " << pMesh->indices.size() << " indices." << std::endl;
	// ��վɵĶ������������
	pMesh->vertices.clear();
	pMesh->indices.clear();

	// ���� simMesh.positions����� Mesh �Ķ�������
	for (size_t i = 0; i < sim_mesh.positions.size(); ++i)
	{
		Vertex vertex;

		// ���ö���λ��
		vertex.Position = glm::vec3(
			static_cast<float>(sim_mesh.positions[i].x),
			static_cast<float>(sim_mesh.positions[i].y),
			static_cast<float>(sim_mesh.positions[i].z));

		// ���÷��ߣ�������ڣ�
		if (i < sim_mesh.normals.size())
		{
			vertex.Normal = glm::vec3(
				static_cast<float>(sim_mesh.normals[i].x),
				static_cast<float>(sim_mesh.normals[i].y),
				static_cast<float>(sim_mesh.normals[i].z));
		}
		else
		{
			vertex.Normal = glm::vec3(0.0f, 0.0f, 0.0f); // Ĭ�Ϸ���
		}

		// �����������꣨������ڣ�
		if (i < sim_mesh.uvs.size())
		{
			vertex.TexCoords = glm::vec2(
				static_cast<float>(sim_mesh.uvs[i].x),
				static_cast<float>(sim_mesh.uvs[i].y));
		}
		else
		{
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
	for (size_t i = 0; i < sim_mesh.indices.size(); ++i)
	{
		pMesh->indices.push_back(static_cast<unsigned int>(sim_mesh.indices[i]));
	}

	// ���� setupMesh ���� VAO/VBO/EBO
	pMesh->updateMesh();
	std::cout << "	After: " << pMesh->vertices.size() << " vertices and " << pMesh->indices.size() << " indices." << std::endl;
	std::cout << "FluidSimulator:: update mesh end." << std::endl;
	return 0;
}

//=====================================================================================
void FluidSimulator::start()
{
	if (sim_paused)
	{
		sim_paused = false;
		std::cout << "FluidSimulator: Start fluid simulation!" << std::endl;
	}
	else
		std::cout << "FluidSimulator: Fluid Simulation Already Started!" << std::endl;
}
void FluidSimulator::pause()
{
	if (!sim_paused)
	{
		sim_paused = true;
		std::cout << "FluidSimulator: Pause fluid simulation!" << std::endl;
	}
	else
		std::cout << "FluidSimulator: Fluid Simulation Already Paused!" << std::endl;
}
void FluidSimulator::reset()
{
	sim_reset = true;
	std::cout << "FluidSimulator: Reset fluid simulation!" << std::endl;
}
void FluidSimulator::advance()
{
	sim_advance = true;
	std::cout << "FluidSimulator: Excute fluid simulation one step!" << std::endl;
}

void FluidSimulator::BindMesh(Mesh *const pm)
{
	if (pm)
	{
		isMeshBound = true;
		pMesh = pm;
		std::cout << "FluidSimulator:: Mesh Bound successfully!" << std::endl;
	}
	else
		std::cout << "FluidSimulator:: Fail to bind Mesh!" << std::endl;
	return;
}
void FluidSimulator::BindMeshSignal(bool *const ps)
{
	if (ps)
		pMeshFinSignal = ps;
}
void FluidSimulator::BindSimSignal(bool *const ps)
{
	if (ps)
		pSimFinSignal = ps;
}

double FluidSimulator::get_scale() const
{
	return _scale;
}
double FluidSimulator::get_time() const
{
	return sim_time;
}
double FluidSimulator::get_time_step() const
{
	return sim_dt;
}
vec3d FluidSimulator::get_grid_offset() const
{
	return sim_grid_offset;
}
vec3s FluidSimulator::get_grid_size() const
{
	return sim_grid_size;
}
vec3d FluidSimulator::get_grid_center() const
{
	return sim_grid_center;
}
double FluidSimulator::get_cell_size() const
{
	return sim_cell_size;
}
vec3d FluidSimulator::get_min_corner() const
{
	return sim_grid_offset;
}
vec3d FluidSimulator::get_max_corner() const
{
	return sim_grid_offset + vec3d(sim_grid_size) * sim_cell_size;
}

std::vector<simulation::particle> FluidSimulator::get_particles()
{
	std::lock_guard<std::mutex> guard(sim_particles_lock);
	return sim_particles;
}
mesher::mesh_t FluidSimulator::get_mesh_t()
{
	std::lock_guard<std::mutex> guard(sim_mesh_lock);
	return sim_mesh;
}
grid3<std::size_t> FluidSimulator::get_grid_occupation()
{
	std::lock_guard<std::mutex> guard(sim_particles_lock);
	return sim_grid_occupation;
}
grid3<vec3d> FluidSimulator::get_grid_velocities()
{
	std::lock_guard<std::mutex> guard(sim_particles_lock);
	return sim_grid_velocities;
}
mesher::mesh_t FluidSimulator::get_room_mesh_t() const
{
	return roomModel.get_mesh();
}

void FluidSimulator::save_mesh_to_obj(const std::string &filepath)
{
	std::lock_guard<std::mutex> guard(sim_mesh_lock);
	std::ofstream fout(filepath);
	sim_mesh.save_obj(fout); // ��������Ϊ .obj �ļ�
	std::cout << "Have exported mesh data to " << filepath << std::endl;
}

void FluidSimulator::save_points_to_txt(const std::string &filepath)
{
	std::vector<vec3d> points;
	{
		std::lock_guard<std::mutex> guard(sim_particles_lock);
		for (const fluid::simulation::particle &p : sim_particles)
		{
			points.emplace_back(p.position); // ��ȡ����λ����Ϣ
		}
	}
	std::ofstream fout(filepath);
	fluid::point_cloud::save_to_naive(fout, points.begin(), points.end()); // ��������λ��
	std::cout << "Have exported points data to points.txt" << std::endl;
}

// ͬ������
void FluidSimulator::wait_until_next_sim(int i)
{
	while (!SimFinSignal)
	{
		continue;
	}
	SimFinSignal = false;
	std::cout << "FluidSimulator:: ";
	if (i >= 0)
		std::cout << "the " << i << "th";
	else
		std::cout << "a new";
	std::cout << " sim time step come." << std::endl;
}
void FluidSimulator::wait_until_next_frame(int i)
{
	while (!MeshFinSignal)
	{
		continue;
	}
	MeshFinSignal = false;
	std::cout << "FluidSimulator:: ";
	if (i >= 0)
		std::cout << "the " << i << "th";
	else
		std::cout << "a new";
	std::cout << " frame come." << std::endl;
}