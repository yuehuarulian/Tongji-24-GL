#include <iostream>  // �����׼���������
#include <fstream>   // �����ļ�����
#include <atomic>    // ����ԭ�Ӳ����⣬���ڶ��̰߳�ȫ
#include <thread>    // �����߳̿�
#include <mutex>     // ���뻥�����⣬���ڶ��߳�ͬ��

#ifdef _WIN32
#ifndef NOMINMAX
#   define NOMINMAX  // 避免 Windows 下的 min/max 宏定义干扰
#endif
#   include <Windows.h>  // ���� Windows ƽ̨��ؿ�
#endif
#include <GL/gl.h>     // OpenGL �⣬����ͼ����Ⱦ
#include <GL/glu.h>    // OpenGL ���߿�
#include <GLFW/glfw3.h>  // GLFW �⣬���ڴ������ںʹ�������

#include <fluid/simulation.h>  // ����ģ����ص�ͷ�ļ�
#include <fluid/data_structures/grid.h>  // �������ݽṹ
#include <fluid/mesher.h>  // ����������
#include <fluid/data_structures/obstacle.h>  // ��̬�ϰ�����
#include <fluid/data_structures/point_cloud.h>  // �������ݽṹ
#include <fluid/math/constants.h>  // ��ѧ����pi

#include <fluid/renderer/path_tracer.h>  // ·��׷����
#include <fluid/renderer/rendering.h>  // ��Ⱦ��غ���
#include <fluid/renderer/bidirectional_path_tracer.h>  // ˫��·��׷����

#include "test_scenes.h"  // ���Գ���ͷ�ļ�
#include "load_model.h"  // ���Գ���ͷ�ļ�

using fluid::vec2d;  // ʹ�� fluid �����ռ��еĶ�ά��������
using fluid::vec2s;  // ʹ�� fluid �����ռ��еĶ�ά������������
using fluid::vec3d;  // ʹ�� fluid �����ռ��е���ά��������
using fluid::vec3s;  // ʹ�� fluid �����ռ��е���ά������������

using namespace fluid::renderer;  // ʹ�� fluid::renderer �����ռ��е�����

// ����һЩԭ�ӱ��������ڿ���ģ��״̬
std::atomic_bool
sim_paused = false,  // ����ģ���Ƿ���ͣ
sim_reset = true,   // ����ģ���Ƿ�����
sim_advance = false;  // ����ģ���Ƿ񵥲�ǰ��

// ���ط���ģ��
fluid::LoadModel roomModel("./source/model/room/overall.obj", 0.1, vec3d(0.0, 0.0, 0.0));

// ���������ģ����ر���
vec3d sim_grid_offset(roomModel.get_offset() - vec3d(1.0, 1.0, 1.0));  // ����ƫ����
vec3s sim_grid_size(roomModel.get_size() + vec3d(2.0, 2.0, 2.0));  // �����С
vec3d sim_grid_center(roomModel.get_offset() + roomModel.get_size() / 2); // ��������
double sim_cell_size = 1.0;  // ����Ԫ��С
std::mutex sim_particles_lock;  // ����ͬ���������ݵĻ�����
std::vector<fluid::simulation::particle> sim_particles;  // ��������
fluid::grid3<std::size_t> sim_grid_occupation;  // ����ռ�����
fluid::grid3<vec3d> sim_grid_velocities;  // �����ٶ�����
std::atomic<std::size_t> sim_config = 5;  // ģ�����ñ��
bool sim_mesh_valid = false;  // ������Ч�Ա�־

//������ģ��ˢ������
fluid::obstacle roomObstacle(roomModel.get_mesh(), sim_cell_size, sim_grid_offset, sim_grid_size);

fluid::semaphore sim_mesher_sema;  // ���ڿ����������ɵ��ź���

// ����ģ��״̬�ĺ���
void update_simulation(const fluid::simulation &sim) {
	// �ռ��µ���������
	std::vector<fluid::simulation::particle> new_particles(sim.particles().begin(), sim.particles().end());

	double energy = 0.0;  // ���ڼ���������
	for (fluid::simulation::particle &p : new_particles) {
		energy += 0.5 * p.velocity.squared_length();  // ����
		energy -= fluid::vec_ops::dot(sim.gravity, p.position);  // ��������
	}
	std::cout << "    total energy: " << energy << "\n";  // ���������

	// �ռ������ռ�����
	fluid::grid3<std::size_t> grid(sim.grid().grid().get_size(), 0);  // ��ʼ������
	for (const auto &particle : sim.particles()) {
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

// ģ���̣߳���������ģ���ִ��
void simulation_thread() {
	fluid::simulation sim;  // ����ģ�����

	sim.resize(sim_grid_size);  // ����ģ�������С
	sim.grid_offset = sim_grid_offset;  // ��������ƫ����
	sim.cell_size = sim_cell_size;  // ��������Ԫ��С
	sim.simulation_method = fluid::simulation::method::apic;  // ����ģ�ⷽ��Ϊ APIC��Affine Particle-In-Cell��
	/*sim.blending_factor = 0.99;*/
	sim.blending_factor = 1.0;  // ���û������
	sim.gravity = vec3d(0.0, -981.0, 0.0);  // ������������

	// ��ÿ��ʱ�䲽֮ǰ�Ļص��������������ʱ�䲽��Ϣ
	sim.pre_time_step_callback = [](double dt) {
		std::cout << "  time step " << dt << "\n";
	};
	// ��ѹ�����֮��Ļص�������������������Ϣ
	sim.post_pressure_solve_callback = [&sim](
		double, std::vector<double> &pressure, double residual, std::size_t iters
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
		for (const fluid::simulation::particle &p : sim.particles()) {
			maxv = std::max(maxv, p.velocity.squared_length());  // ��ȡ����ٶ�
		}
		std::cout << "    max particle velocity = " << std::sqrt(maxv) << "\n";
	};
	// ģ����ѭ��
	while (true) {
		if (sim_reset) {  // �����Ҫ����
			sim.particles().clear();  // �������
			// ���ù��嵥Ԫ
			sim.grid().grid().for_each(
				[](vec3s, fluid::mac_grid::cell &cell) {
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
			switch (sim_config) {
			case 0:
				sim.seed_box(sim_grid_center - vec3d(5, 5, 5), vec3d(10, 10, 10));
				//sim.seed_box(vec3d(15, 15, 15), vec3d(20, 20, 20));  // ����һ�����������������
				break;
			case 1:
				sim.seed_sphere(sim_grid_center, 5.0);
				//sim.seed_sphere(vec3d(25.0, 25.0, 25.0), 5.0);  // ����һ��������������
				break;
			case 2:
				sim.seed_sphere(vec3d(25, 44, 25), 5);  // ����һ��������������
				sim.seed_box(vec3d(0, 0, 0), vec3d(50, 15, 50));  // ����һ����������������
				break;
			case 3:
				sim.seed_box(vec3d(0, 0, 0), vec3d(10, 50, 50));  // ����һ���ϳ�����������������
				break;
			case 4:
				{
					// ����һ������Դ�������ٶȺ�λ��
					auto source = std::make_unique<fluid::source>();
					for (std::size_t x = 1; x < 5; ++x) {
						for (std::size_t y = 25; y < 35; ++y) {
							for (std::size_t z = 20; z < 30; ++z) {
								source->cells.emplace_back(x, y, z);  // ��������Դ�ĵ�Ԫλ��
							}
						}
					}
					source->velocity = vec3d(200.0, 0.0, 0.0);  // ����Դ���ٶ�
					source->coerce_velocity = true;  // ǿ������Դ���ٶ�
					sim.sources.emplace_back(std::move(source));  // ��Դ���ӵ�ģ����

					// ���������ϰ���
					sim.grid().grid().for_each_in_range_unchecked(
						[](vec3s cell, fluid::mac_grid::cell &c) {
							vec3d diff = vec3d(cell) + 0.5 * vec3d(sim_cell_size, sim_cell_size, sim_cell_size);
							diff -= vec3d(25.0, 25.0, 25.0);  // ����λ�ò�
							if (diff.squared_length() < 100.0) {  // ��������巶Χ��
								c.cell_type = fluid::mac_grid::cell::type::solid;  // ����Ԫ����Ϊ����
							}
						},
						vec3s(15, 15, 15), vec3s(35, 35, 35)  // ��������ı߽�
							);
				}
				break;
			case 5:
				//����ڲ�����
				std::cout << "Start filling interior area..." << std::endl;
				/*sim.seed_area(sim_grid_offset, vec3d(sim_grid_size),
					[&](vec3d pos) {
						return
							pos.y < -double(sim_grid_size.x) / 6 + (pos.x + pos.z) / 5 &&
							(pos.x - sim_grid_center.x) * (pos.x - sim_grid_center.x) + (pos.z - sim_grid_center.z) * (pos.z - sim_grid_center.z) >= sim_grid_size.z * sim_grid_size.z / 16 &&
							roomObstacle.is_cell_inside(pos);
					},
					vec3d(100.0, 0.0, 0.0)
				);*/
				sim.seed_area(sim_grid_offset, vec3d(sim_grid_size),
					[&](vec3d pos) {
						return
							pos.y < -double(sim_grid_size.x) / 6 &&
							roomObstacle.is_cell_inside(pos);
					},
					vec3d(0.0, 0.0, 0.0)
				);
				sim.seed_area(sim_grid_offset, vec3d(sim_grid_size),
					[&](vec3d pos) {
						return
							pos.y > -double(sim_grid_size.x) / 6 &&
							pos.y < -double(sim_grid_size.x) / 7 &&
							pos.x > 0 &&
							roomObstacle.is_cell_inside(pos);
					},
					vec3d(0.0, 0.0, 0.0)
				);
				sim.seed_area(sim_grid_offset, vec3d(sim_grid_size),
					[&](vec3d pos) {
						return
							pos.y > -double(sim_grid_size.x) / 6 &&
							pos.y < -double(sim_grid_size.x) / 7 &&
							pos.x < 0 &&
							roomObstacle.is_cell_inside(pos);
					},
					vec3d(400.0, 0.0, 0.0)
				);
				std::cout << "Starting seting liquid source..." << std::endl;
				// ����һ������Դ�������ٶȺ�λ��
				/*auto source = std::make_unique<fluid::source>();
				for (std::size_t x = 1; x < 5; ++x) {
					for (std::size_t y = sim_grid_size.y / 2 - sim_grid_size.y / 20; y < sim_grid_size.y / 2 + sim_grid_size.y / 20; ++y) {
						for (std::size_t z = sim_grid_size.z / 2 - sim_grid_size.y / 20; z < sim_grid_size.z / 2 + sim_grid_size.y / 20; ++z) {
							if (roomObstacle.is_cell_inside(vec3s(x, y, z)))
								source->cells.emplace_back(x, y, z);  // ��������Դ�ĵ�Ԫλ��
						}
					}
				}
				source->velocity = vec3d(400.0, 0.0, 0.0);  // ����Դ���ٶ�
				source->coerce_velocity = true;  // ǿ������Դ���ٶ�
				sim.sources.emplace_back(std::move(source));  // ��Դ���ӵ�ģ����
				*/
				break;
			}
			std::cout << "Finish initializing liquid state." << std::endl;
			std::cout << std::endl;

			// ���ÿռ��ϣ�����ڸ�������λ�ú�״̬
			sim.reset_space_hash();

			update_simulation(sim);  // ����ģ��״̬
			sim_reset = false;  // �������
		}

		if (!sim_paused) {  // ���δ��ͣ
			std::cout << "update\n";
			sim.update(1.0 / 60.0);  // ����һ��ʱ�䲽����ģ�����
			update_simulation(sim);  // ����ģ��״̬
		} else if (sim_advance) {  // �������Ϊ����ǰ��
			sim_advance = false;
			sim.time_step();  // ����һ��ʱ�䲽��
			update_simulation(sim);  // ����ģ��״̬
		}
		std::cout << "one sim thread end." << std::endl;
	}
}

// ���������̣߳����ڽ��������������Թ���Ⱦ
std::mutex sim_mesh_lock;  // �����������ڱ�����������
fluid::mesher::mesh_t sim_mesh;  // ��������

void mesher_thread() {
	while (true) {
		sim_mesher_sema.wait();  // �ȴ��ź�����ȷ��������Ҫ����
		std::vector<vec3d> particles;
		{
			std::lock_guard<std::mutex> lock(sim_particles_lock);  // �����Է�����������
			if (sim_mesh_valid) {
				continue;  // �������������Ч������������
			}
			for (const fluid::simulation::particle &p : sim_particles) {
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
		}
	}
}


scene rend_scene;  // ��Ⱦ����
camera rend_cam;  // ��Ⱦ�����
image<spectrum> rend_accum(vec2s(400, 400));  // �ۻ�ͼ������·��׷��
image<fluid::vec3<std::uint8_t>> rend_image;  // ��Ⱦͼ��
pcg32 rend_random;  // �����������������·��׷��
bidirectional_path_tracer rend_tracer;  // ˫��·��׷����
std::size_t rend_spp = 0;  // ÿ���ز�������

// ������Ⱦ�����������
void update_scene(scene&& sc, camera& cam) {
	rend_scene = std::move(sc);  // ���³���
	rend_scene.finish();  // ��ɳ�����ʼ��
	rend_cam = cam;  // ���������
	rend_accum = image<spectrum>(rend_accum.pixels.get_size());  // �����ۻ�ͼ��
	rend_spp = 0;  // ����ÿ���ز�������
}

enum class particle_visualize_mode : unsigned char {
	none,
	velocity_direction,
	velocity_magnitude,
	maximum
};
enum class mesh_visualize_mode : unsigned char {
	none,
	transparent,
	maximum
};

bool
rotating = false,  // �����������ת
rendering = false,  // �����Ƿ���Ⱦ
draw_particles = true,  // �����Ƿ���ʾ����
draw_cells = false,  // �����Ƿ���ʾ����Ԫ
draw_faces = false,  // �����Ƿ���ʾ������
draw_mesh = true,  // �����Ƿ���ʾ����
draw_apic_debug = true,  // �����Ƿ���ʾ APIC ������Ϣ
draw_render_preview = true;  // �����Ƿ���ʾ��ȾԤ��
vec2d mouse, rotation;  // ���λ�ú���ת�Ƕ�
particle_visualize_mode particle_vis = particle_visualize_mode::none;  // ���ӿ��ӻ�ģʽ
mesh_visualize_mode mesh_vis = mesh_visualize_mode::none;  // ������ӻ�ģʽ
//double camera_distance = -70.0;  // ������볡���ľ���
double camera_distance = -double(sim_grid_size.z) * 2;  // ������볡���ľ���
GLuint render_preview_texture = 0;  // ��ȾԤ������

// �����¼��ص����������ڴ����û�����
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action == GLFW_PRESS) {
		switch (key) {
		case GLFW_KEY_ENTER:
			sim_paused = !sim_paused;  // �л�ģ����ͣ״̬
			std::cout << "Press Enter shift sim_paused to " << sim_paused << std::endl;
			break;
		case GLFW_KEY_R:
			sim_reset = true;  // ����ģ������
			std::cout << "Press R shift sim_reset to " << sim_reset << std::endl;
			break;
		case GLFW_KEY_SPACE:
			sim_advance = true;  // ����ǰ��ģ��
			std::cout << "Press SPACE shift sim_advance to " << sim_advance << std::endl;
			break;

		case GLFW_KEY_P:
			draw_particles = !draw_particles;  // �л�������ʾ
			break;
		case GLFW_KEY_C:
			draw_cells = !draw_cells;  // �л�����Ԫ��ʾ
			break;
		case GLFW_KEY_F:
			draw_faces = !draw_faces;  // �л���������ʾ
			break;
		case GLFW_KEY_M:
			draw_mesh = !draw_mesh;  // �л�������ʾ
			break;
		case GLFW_KEY_A:
			draw_apic_debug = !draw_apic_debug;  // �л� APIC ������ʾ
			break;
		
		case GLFW_KEY_V:
			draw_render_preview = !draw_render_preview;  // �л���ȾԤ��
			std::cout << "Press V shift draw_render_preview to " << draw_render_preview << std::endl;
			break;
		case GLFW_KEY_S:
			rendering = !rendering;  // �л���Ⱦ״̬
			std::cout << "Press S shift rendering to " << rendering << std::endl;
			break;
		// ���ӻ�ģʽ�л�
		case GLFW_KEY_F1:
			particle_vis = static_cast<particle_visualize_mode>(static_cast<unsigned char>(particle_vis) + 1);
			if (particle_vis == particle_visualize_mode::maximum) {
				particle_vis = particle_visualize_mode::none;
			}
			break;
		case GLFW_KEY_F2:
			mesh_vis = static_cast<mesh_visualize_mode>(static_cast<unsigned char>(mesh_vis) + 1);
			if (mesh_vis == mesh_visualize_mode::maximum) {
				mesh_vis = mesh_visualize_mode::none;
			}
			break;
		// ��������
		case GLFW_KEY_F3:
			std::cout << "Press F3 to export mesh data to mesh.obj" << std::endl;
			{
				std::lock_guard<std::mutex> guard(sim_mesh_lock);
				std::ofstream fout("mesh.obj");
				sim_mesh.save_obj(fout);  // ��������Ϊ .obj �ļ�
				std::cout << "Have exported mesh data to mesh.obj" << std::endl;
			}
			break;
		case GLFW_KEY_F4:
			std::cout << "Press F4 to export points data to points.txt" << std::endl;
			{
				std::vector<vec3d> points;
				{
					std::lock_guard<std::mutex> guard(sim_particles_lock);
					for (const fluid::simulation::particle &p : sim_particles) {
						points.emplace_back(p.position);  // ��ȡ����λ����Ϣ
					}
				}
				std::ofstream fout("points.txt");
				fluid::point_cloud::save_to_naive(fout, points.begin(), points.end());  // ��������λ��
				std::cout << "Have exported points data to points.txt" << std::endl;
			}
			break;
		// ѡ�񳡾�
		case GLFW_KEY_1:
			{
				auto [sc, cam] = cornell_box_two_lights(rend_accum.aspect_ratio());
				update_scene(std::move(sc), cam);  // ������������Դ�� Cornell Box ����
			}
			break;

		case GLFW_KEY_2:
			{
				auto [sc, cam] = glass_ball_box(rend_accum.aspect_ratio());
				update_scene(std::move(sc), cam);  // ���ز����򳡾�
			}
			break;

		case GLFW_KEY_0:
			{
				fluid::mesher::mesh_t mesh;
				{
					std::lock_guard<std::mutex> guard(sim_mesh_lock);  // �����Ա�����������
					mesh = sim_mesh;  // ������ǰ����������
				}
				mesh.reverse_face_directions();  // ��ת������ķ���
				mesh.generate_normals();  // ����������
				vec3d min = sim_grid_offset;
				vec3d max = min + vec3d(sim_grid_size) * sim_cell_size;
				auto [sc, cam] = fluid_box(min, max, 30.0 * fluid::constants::pi / 180.0, rend_accum.aspect_ratio());

				{ // ����ˮ��
					entity_info info;
					auto &water = info.mat.value.emplace<materials::specular_transmission>();
					water.index_of_refraction = 1.7;  // ����������
					water.skin.modulation = spectrum::identity;
					sc.add_mesh_entity(mesh, fluid::rmat3x4d::identity(), info);  // ����ˮ���񵽳���
				}

				switch (sim_config) {
				case 4:
					{ // ������������
						entity_info info;
						auto &lambert = info.mat.value.emplace<materials::lambertian_reflection>();
						lambert.reflectance.modulation = spectrum::from_rgb(vec3d(0.2, 0.5, 0.8));
						primitive prim;
						primitives::sphere_primitive sphere;
						sphere.set_transformation(fluid::transform::scale_rotate_translate(
							vec3d(10.0, 10.0, 10.0), vec3d(), vec3d(25.0, 25.0, 25.0)
						));
						sc.add_primitive_entity(sphere, info);  // ��������ʵ�嵽����
					}
					break;
				}

				update_scene(std::move(sc), cam);  // ���³����������
			}
			break;

		// ���ò�ͬ������ģ������
		case GLFW_KEY_5:
			sim_config = 4;
			sim_reset = true;
			break;

		case GLFW_KEY_6:
			sim_config = 3;
			sim_reset = true;
			break;

		case GLFW_KEY_7:
			sim_config = 2;
			sim_reset = true;
			break;

		case GLFW_KEY_8:
			sim_config = 1;
			sim_reset = true;
			break;

		case GLFW_KEY_9:
			sim_config = 0;
			sim_reset = true;
			break;

		// ������Ⱦ���
		case GLFW_KEY_F5:
			{
				std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
				image<spectrum> img = render_naive<true>(
					[](ray r, pcg32 &rnd) {
						return rend_tracer.incoming_light(rend_scene, r, rnd);
					},
					rend_cam, 2 * rend_accum.pixels.get_size(), 400, rend_random
						);
				img.save_ppm(
					"test.ppm",
					[](spectrum pixel) {
						vec3d rgb = pixel.to_rgb() * 255.0;
						return fluid::vec_ops::apply<fluid::vec3<std::uint8_t>>(
							[](double v) {
								return static_cast<std::uint8_t>(std::clamp(v, 0.0, 255.0));
							},
							rgb
								);
					}
				);
				std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
				std::cout << "render: " << std::chrono::duration<double>(t2 - t1).count() << "s\n";
			}
			break;

		case GLFW_KEY_F6:
			{
				rend_accum.save_ppm(
					"test.ppm",
					[](spectrum pixel) {
						vec3d rgb = pixel.to_rgb() / static_cast<double>(rend_spp);
						return fluid::vec_ops::apply<fluid::vec3<std::uint8_t>>(
							[](double v) {
								return static_cast<std::uint8_t>(std::clamp(v * 255.0, 0.0, 255.0));
							},
							rgb
								);
					}
				);
			}
			break;
		}
	}
}

// ��갴ť�¼��ص����������ڿ����������ת
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_LEFT) {  // ����Ƿ�Ϊ���
		rotating = action == GLFW_PRESS;  // �������ʱ��ʼ��ת
	}
}

// ���λ�ûص����������ڸ������������ת�Ƕ�
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
	vec2d new_mouse(xpos, ypos);
	if (rotating) {  // ���������ת״̬
		rotation += new_mouse - mouse;  // ������ת�Ƕ�
		rotation.y = std::clamp(rotation.y, -90.0, 90.0);  // ������ֱ�������ת�Ƕ�
	}
	mouse = new_mouse;  // �������λ��
}

// ���ڴ�С�����ص����������ڵ�����ͼ
void resize_callback(GLFWwindow *window, int width, int height) {
	glViewport(0, 0, width, height);  // �����ӿڴ�С
	glMatrixMode(GL_PROJECTION);  // ����ΪͶӰ����ģʽ
	glLoadIdentity();  // ����ͶӰ����
	gluPerspective(60.0, width / static_cast<double>(height), 0.1, 1000.0);  // ����͸��ͶӰ
}

// �����¼��ص����������ڵ���������볡���ľ���
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
	camera_distance += yoffset;  // ���ݹ�������������������
}

// obj��Ⱦ����
void render_obj_mesh(const scene::mesh_t& mesh) {
	//glPushMatrix();

	// ʹ�� OpenGL ����������
	//glBegin(GL_TRIANGLES);
	for (size_t i = 0; i < mesh.indices.size(); i += 3) {
		// ���÷��ߣ�������ڣ�
		if (!mesh.normals.empty()) {
			const auto& normal = mesh.normals[mesh.indices[i]];
			glNormal3f(float(normal.x), float(normal.y), float(normal.z));
		}

		// ���ƶ���
		for (int j = 0; j < 3; ++j) {
			const auto& pos = mesh.positions[mesh.indices[i + j]];
			glVertex3f(static_cast<GLfloat>(pos.x), static_cast<GLfloat>(pos.y), static_cast<GLfloat>(pos.z));
		}
	}
	//glEnd();

	//glPopMatrix();
}

// ���������
int main() {
	if (!glfwInit()) {  // ��ʼ�� GLFW
		return -1;
	}

	GLFWwindow *window = glfwCreateWindow(800, 600, "libfluid", nullptr, nullptr);  // ��������
	if (!window) {  // ������ڴ���ʧ��
		glfwTerminate();
		return -1;
	}
	// ���ø��ֻص��������������̡���ꡢ���ڴ�С���¼�
	glfwSetKeyCallback(window, key_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetWindowSizeCallback(window, resize_callback);
	glfwSetScrollCallback(window, scroll_callback);

	glfwMakeContextCurrent(window);  // ���õ�ǰ����Ϊ OpenGL ������

	// ���� OpenGL �Ļ�Ϻ�������Ⱦ����
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_2D);

	// ��ʼ���ӿں�ͶӰ����
	int width, height;
	glfwGetWindowSize(window, &width, &height);
	resize_callback(window, width, height);

	// ����ģ������������߳�
	std::thread sim_thread(simulation_thread);
	sim_thread.detach();  // ��ģ���߳���Ϊ����ģʽ
	std::thread mesh_thread(mesher_thread);
	mesh_thread.detach();  // �����������߳���Ϊ����ģʽ

	// ���� OpenGL ������������ȾԤ��
	glGenTextures(1, &render_preview_texture);

	// ����Ⱦѭ��
	while (!glfwWindowShouldClose(window)) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  // �����ɫ����Ȼ�����

		glMatrixMode(GL_MODELVIEW);  // ����Ϊģ����ͼ����ģʽ
		glLoadIdentity();  // ����ģ����ͼ����
		glTranslated(0, 0, camera_distance);  // �������������
		glRotated(rotation.y, 1, 0, 0);  // Ӧ���������ֱ��ת
		glRotated(rotation.x, 0, 1, 0);  // Ӧ�������ˮƽ��ת
		//glTranslatef(-25, -25, -25); // ���������λ�ã�ʹ��������
		glTranslatef(-GLfloat(sim_grid_center.x), -GLfloat(sim_grid_center.y), -GLfloat(sim_grid_center.z));

		glEnable(GL_DEPTH_TEST);  // ������Ȳ���

		// �����������������
		glBegin(GL_LINES);
		vec3d
			min_corner = sim_grid_offset,
			max_corner = sim_grid_offset + vec3d(sim_grid_size) * sim_cell_size;

		// ���������ᣨ��ɫ����ɫ����ɫ�ֱ��ʾ X��Y��Z �ᣩ
		glColor3d(0.5, 0.0, 0.0);
		glVertex3d(min_corner.x, min_corner.y, min_corner.z);
		glVertex3d(max_corner.x, min_corner.y, min_corner.z);

		glColor3d(0.0, 0.5, 0.0);
		glVertex3d(min_corner.x, min_corner.y, min_corner.z);
		glVertex3d(min_corner.x, max_corner.y, min_corner.z);

		glColor3d(0.0, 0.0, 0.5);
		glVertex3d(min_corner.x, min_corner.y, min_corner.z);
		glVertex3d(min_corner.x, min_corner.y, max_corner.z);

		// ��������߿�
		glColor3d(0.3, 0.3, 0.3);

		glVertex3d(max_corner.x, min_corner.y, min_corner.z);
		glVertex3d(max_corner.x, max_corner.y, min_corner.z);

		glVertex3d(max_corner.x, min_corner.y, min_corner.z);
		glVertex3d(max_corner.x, min_corner.y, max_corner.z);

		glVertex3d(min_corner.x, max_corner.y, min_corner.z);
		glVertex3d(max_corner.x, max_corner.y, min_corner.z);

		glVertex3d(min_corner.x, max_corner.y, min_corner.z);
		glVertex3d(min_corner.x, max_corner.y, max_corner.z);

		glVertex3d(min_corner.x, min_corner.y, max_corner.z);
		glVertex3d(max_corner.x, min_corner.y, max_corner.z);

		glVertex3d(min_corner.x, min_corner.y, max_corner.z);
		glVertex3d(min_corner.x, max_corner.y, max_corner.z);

		glVertex3d(max_corner.x, max_corner.y, min_corner.z);
		glVertex3d(max_corner.x, max_corner.y, max_corner.z);

		glVertex3d(max_corner.x, min_corner.y, max_corner.z);
		glVertex3d(max_corner.x, max_corner.y, max_corner.z);

		glVertex3d(min_corner.x, max_corner.y, max_corner.z);
		glVertex3d(max_corner.x, max_corner.y, max_corner.z);

		// ���Ʒ�������
		render_obj_mesh(roomModel.get_mesh());

		//TODO�����ֹ����Һ��

		glEnd();

		if (draw_mesh) {
			glEnable(GL_CULL_FACE);  // �������޳�����ֹ�������
			glCullFace(GL_FRONT);  // �����޳�������棬�޳����������ȾЧ��

			glEnable(GL_LIGHTING);  // ���ù���

			glEnable(GL_LIGHT0);  // ���õ�һ����Դ
			GLfloat color1[]{ 0.6f, 0.6f, 0.6f, 1.0f };  // ��Դ1����ɫ
			glLightfv(GL_LIGHT0, GL_DIFFUSE, color1);  // ���ù�Դ1���������
			glLightfv(GL_LIGHT0, GL_SPECULAR, color1);  // ���ù�Դ1�ľ��淴���
			GLfloat dir1[]{ -1, -1, -1, 0 };  // ��Դ1�ķ���
			glLightfv(GL_LIGHT0, GL_POSITION, dir1);  // ���ù�Դ1��λ��

			glEnable(GL_LIGHT1);  // ���õڶ�����Դ
			GLfloat color2[]{ 0.6f, 0.6f, 0.6f, 1.0f };  // ��Դ2����ɫ
			glLightfv(GL_LIGHT1, GL_DIFFUSE, color2);  // ���ù�Դ2���������
			glLightfv(GL_LIGHT1, GL_SPECULAR, color2);  // ���ù�Դ2�ľ��淴���
			GLfloat dir2[]{ 1, -1, 1, 0 };  // ��Դ2�ķ���
			glLightfv(GL_LIGHT1, GL_POSITION, dir2);  // ���ù�Դ2��λ��

			glEnable(GL_COLOR_MATERIAL);  // ������ɫ���ٲ���
			glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT | GL_DIFFUSE);  // ���ò��ʵĻ�������������

			glBegin(GL_TRIANGLES);  // ��ʼ��������������
			switch (mesh_vis) {
			case mesh_visualize_mode::none:  // ��������ʾģʽ
				glColor4d(1.0, 1.0, 1.0, 1.0);  // ����������ɫΪ��ɫ��͸��
				break;
			case mesh_visualize_mode::transparent:  // ͸��ģʽ
				glColor4d(0.5, 0.5, 1.0, 0.2);  // ����������ɫΪ��͸����ɫ
				break;
			}
			{
				std::lock_guard<std::mutex> guard(sim_mesh_lock);  // ����������������
				for (std::size_t i = 0; i < sim_mesh.indices.size(); ++i) {
					vec3d v = sim_mesh.positions[sim_mesh.indices[i]];  // ��ȡ���񶥵�
					vec3d n = sim_mesh.normals[sim_mesh.indices[i]];  // ��ȡ������
					glNormal3d(n.x, n.y, n.z);  // ���ö��㷨��
					glVertex3d(v.x, v.y, v.z);  // ���ö�������
				}
			}
			glEnd();  // ���������λ���

			glDisable(GL_LIGHTING);  // ���ù���
		}

		glDisable(GL_DEPTH_TEST);

		if (draw_faces) {
			glBegin(GL_LINES);  // ��ʼ�����߶�
			glColor3d(1.0, 0.0, 0.0);  // �����߶���ɫΪ��ɫ
			double half_cell = 0.5 * sim_cell_size;  // ��Ԫ���С��һ��
			{
				std::lock_guard<std::mutex> guard(sim_particles_lock);  // ����������������
				for (std::size_t z = 0; z < sim_grid_velocities.get_size().z; ++z) {
					for (std::size_t y = 0; y < sim_grid_velocities.get_size().y; ++y) {
						for (std::size_t x = 0; x < sim_grid_velocities.get_size().x; ++x) {
							vec3d
								face_outer = sim_grid_offset + vec3d(vec3s(x + 1, y + 1, z + 1)) * sim_cell_size,
								face_half = face_outer - vec3d(half_cell, half_cell, half_cell),
								vel = sim_grid_velocities(x, y, z) * 0.001;

							// �����ٶȷ�������ٶ�ʸ����ͷ
							if (vel.x > 0.0) {
								glColor4d(1.0, 0.0, 0.0, 0.3);
							} else {
								glColor4d(0.0, 1.0, 1.0, 0.3);
							}
							glVertex3d(face_outer.x, face_half.y, face_half.z);
							glVertex3d(face_outer.x + vel.x, face_half.y, face_half.z);

							if (vel.y > 0.0) {
								glColor4d(0.0, 1.0, 0.0, 0.3);
							} else {
								glColor4d(1.0, 0.0, 1.0, 0.3);
							}
							glVertex3d(face_half.x, face_outer.y, face_half.z);
							glVertex3d(face_half.x, face_outer.y + vel.y, face_half.z);

							if (vel.z > 0.0) {
								glColor4d(0.0, 0.0, 1.0, 0.3);
							} else {
								glColor4d(1.0, 1.0, 0.0, 0.3);
							}
							glVertex3d(face_half.x, face_half.y, face_outer.z);
							glVertex3d(face_half.x, face_half.y, face_outer.z + vel.z);
						}
					}
				}
			}
			glEnd();
		}

		if (draw_particles) {
			glBegin(GL_POINTS);  // ��ʼ���Ƶ�
			{
				std::lock_guard<std::mutex> guard(sim_particles_lock);  // ����������������

				double max_vel = 0.0;
				for (fluid::simulation::particle &p : sim_particles) {
					max_vel = std::max(max_vel, p.velocity.squared_length());  // �ҵ�����ٶ�ֵ
				}
				max_vel = std::sqrt(max_vel);

				for (fluid::simulation::particle &p : sim_particles) {
					vec3d pos = p.position;  // ��ȡ����λ��
					switch (particle_vis) {
					case particle_visualize_mode::none:
						glColor4d(1.0, 1.0, 1.0, 0.3);  // ������������ʾ����Ϊ��͸����ɫ
						break;
					case particle_visualize_mode::velocity_direction:
						{
							vec3d vel = fluid::vec_ops::apply<vec3d>(
								[](double v) {
									v /= 1.0;
									if (v > -1.0 && v < 1.0) {
										v = v < 0.0 ? -1.0 : 1.0;
									}
									return std::clamp(std::log(v) + 0.5, 0.0, 1.0);
								}, p.velocity
							);
							glColor4d(vel.x, vel.y, vel.z, 0.3);  // ʹ���ٶȷ������ɫ
						}
						break;
					case particle_visualize_mode::velocity_magnitude:
						{
							double c = p.velocity.length() / max_vel;  // ���������ٶȵ���Դ�С
							glColor4d(1.0, 1.0, 1.0, c * 0.9 + 0.1);  // �����ٶȴ�С����͸����
						}
						break;
					default:
						glColor4d(1.0, 0.0, 0.0, 1.0);  // Ĭ����ʾΪ��͸����ɫ
						break;
					}
					glVertex3d(pos.x, pos.y, pos.z);  // ��������λ��
				}
			}
			glEnd();
		}

		if (draw_apic_debug) {
			glBegin(GL_LINES);  // ��ʼ�����߶�
			{
				std::lock_guard<std::mutex> guard(sim_particles_lock);  // ����������������
				for (fluid::simulation::particle &p : sim_particles) {
					double mul = 0.01;  // ��������
					vec3d
						pos = p.position,  // ����λ��
						pcx = pos + p.cx * mul,  // APIC ʸ�� X ����
						pcy = pos + p.cy * mul,  // APIC ʸ�� Y ����
						pcz = pos + p.cz * mul;  // APIC ʸ�� Z ����

					// ���� APIC ʸ�����Բ�ͬ��ɫ���� X��Y �� Z ����
					glColor4d(1.0, 0.0, 0.0, 1.0);
					glVertex3d(pos.x, pos.y, pos.z);
					glVertex3d(pcx.x, pcx.y, pcx.z);

					glColor4d(0.0, 1.0, 0.0, 1.0);
					glVertex3d(pos.x, pos.y, pos.z);
					glVertex3d(pcy.x, pcy.y, pcy.z);

					glColor4d(0.0, 0.0, 1.0, 1.0);
					glVertex3d(pos.x, pos.y, pos.z);
					glVertex3d(pcz.x, pcz.y, pcz.z);
				}
			}
			glEnd();  // �����߶λ���
		}

		if (draw_cells) {
			glBegin(GL_POINTS);
			glColor4d(0.0, 1.0, 0.0, 1.0);
			{
				std::lock_guard<std::mutex> guard(sim_particles_lock);
				for (std::size_t z = 0; z < sim_grid_occupation.get_size().z; ++z) {
					for (std::size_t y = 0; y < sim_grid_occupation.get_size().y; ++y) {
						for (std::size_t x = 0; x < sim_grid_occupation.get_size().x; ++x) {
							if (sim_grid_occupation(x, y, z) > 0) {
								vec3d pos = sim_grid_offset + sim_cell_size * (vec3d(vec3s(x, y, z)) + vec3d(0.5, 0.5, 0.5));
								glVertex3d(pos.x, pos.y, pos.z);
							}
						}
					}
				}
			}
			glEnd();
		}

		if (draw_render_preview) {
			glDisable(GL_DEPTH_TEST);  // ������Ȳ���
			glDisable(GL_CULL_FACE);  // �������޳�

			int width, height;
			glfwGetWindowSize(window, &width, &height);  // ��ȡ��ǰ���ڵĿ��Ⱥ͸߶�

			glMatrixMode(GL_PROJECTION);  // �л���ͶӰ����
			glPushMatrix();  // ���浱ǰͶӰ����
			glLoadIdentity();  // ���õ�ǰͶӰ����
			glOrtho(0.0, width, height, 0.0, -1.0, 1.0);  // ��������ͶӰ

			glMatrixMode(GL_MODELVIEW);  // �л���ģ����ͼ����
			glPushMatrix();  // ���浱ǰģ����ͼ����
			glLoadIdentity();  // ����ģ����ͼ����

			// ������������Ԥ����Ⱦ
			glBindTexture(GL_TEXTURE_2D, render_preview_texture);  // ����ȾԤ��������
			vec2s sz = rend_image.pixels.get_size();  // ��ȡ��Ⱦͼ��ĳߴ�

			if (rendering) {  // ���������Ⱦ
				// accumulate samples
				std::size_t frame_spp = 1;  // ��ǰ֡��ÿ���ز�����
				auto t1 = std::chrono::high_resolution_clock::now();  // ��¼��Ⱦ��ʼʱ��
				accumulate_naive(
					[&](ray r, pcg32 &rnd) {
						return rend_tracer.incoming_light(rend_scene, r, rnd);  // ���������
					},
					rend_accum, rend_cam, frame_spp, rend_random
						);
				rend_spp += frame_spp;  // �����ܵĲ�����

				auto t2 = std::chrono::high_resolution_clock::now();  // ��¼��Ⱦ����ʱ��
				std::cout <<
					"sample time: " << std::chrono::duration<double>(t2 - t1).count() << "s / " <<
					frame_spp << " sample(s)\n";  // �����ǰ֡�Ĳ���ʱ��
				std::cout << "total spp: " << rend_spp << "\n";  // ����ܲ�����

				// copy to image
				if (rend_image.pixels.get_size() != rend_accum.pixels.get_size()) {
					rend_image = image<fluid::vec3<std::uint8_t>>(rend_accum.pixels.get_size());
				}
				for (std::size_t y = 0; y < rend_image.pixels.get_size().y; ++y) {
					for (std::size_t x = 0; x < rend_image.pixels.get_size().x; ++x) {
						vec3d color = rend_accum.pixels(x, y).to_rgb() / static_cast<double>(rend_spp);  // ����ÿ������ɫ
						rend_image.pixels(x, y) = fluid::vec_ops::apply<fluid::vec3<std::uint8_t>>(
							[](double v) {
								return static_cast<std::uint8_t>(std::clamp(v * 255.0, 0.0, 255.0));
							},
							color
								);
					}
				}
				// copy to opengl
				// ���� OpenGL ��������
				glTexImage2D(
					GL_TEXTURE_2D, 0, GL_RGB,
					int(rend_image.pixels.get_size().x), int(rend_image.pixels.get_size().y), 0, GL_RGB, GL_UNSIGNED_BYTE,
					&rend_image.pixels[0]
				);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			}

			double scale = std::min(width / static_cast<double>(sz.x), height / static_cast<double>(sz.y));

			glColor3d(1.0, 1.0, 1.0);
			glBegin(GL_TRIANGLE_STRIP);

			glTexCoord2d(0.0, 0.0);
			glVertex2d(0.0, 0.0);

			glTexCoord2d(1.0, 0.0);
			glVertex2d(sz.x * scale, 0.0);

			glTexCoord2d(0.0, 1.0);
			glVertex2d(0.0, sz.y * scale);

			glTexCoord2d(1.0, 1.0);
			glVertex2d(sz.x * scale, sz.y * scale);

			glEnd();

			// cleanup
			glBindTexture(GL_TEXTURE_2D, 0);

			glMatrixMode(GL_MODELVIEW);
			glPopMatrix();
			glMatrixMode(GL_PROJECTION);
			glPopMatrix();

			glEnable(GL_CULL_FACE);  // �����������޳�
			glEnable(GL_DEPTH_TEST);  // ����������Ȳ���
		}

		glfwSwapBuffers(window);  // ������������������ʾ
		glfwPollEvents();  // ���������Ѵ������¼�
	}

	glfwTerminate();  // �ͷ� GLFW ��Դ����ȫ�˳�����
	return 0;
}
