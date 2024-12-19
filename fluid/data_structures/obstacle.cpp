#include "fluid/data_structures/obstacle.h"

/// \file
/// Implementation of obstacles.

#include <iostream>//debug

namespace fluid {
	obstacle::obstacle(mesh_t m, double cell_size, vec3d ref_grid_offset, vec3s ref_grid_size)
		: obstacle_mesh(std::move(m)) {

		std::cout << "Initializing obstacle with mesh data..." << std::endl;

		// 获取障碍物网格的边界框
		auto [rmin, rmax] = voxelizer::get_bounding_box(
			obstacle_mesh.positions.begin(), obstacle_mesh.positions.end()
		);

		//voxelizer vox;
		// 调整体素网格大小和位置，以适应障碍物边界框
		vec3i offset = vox.resize_reposition_grid_constrained(rmin, rmax, cell_size, ref_grid_offset);
		//std::cout << '(' << offset.x << ", " << offset.y << ", " << offset.z << ')' << std::endl;
		
		// 体素化障碍物网格表面并标记外部体素单元
		std::cout << "Starting mesh surface voxelization..." << std::endl;
		vox.voxelize_mesh_surface(obstacle_mesh);
		std::cout << "Mesh surface voxelization completed." << std::endl;

		std::cout << "Marking exterior voxels..." << std::endl;
		vox.mark_exterior();
		std::cout << "Exterior marking completed." << std::endl;

		// 获取障碍物与参考网格的重叠范围
		auto [min_coord, max_coord] = vox.get_overlapping_cell_range(offset, ref_grid_size);
		//std::cout << '(' << min_coord.x << ", " << min_coord.y << ", " << min_coord.z << ')' << std::endl;
		//std::cout << '(' << max_coord.x << ", " << max_coord.y << ", " << max_coord.z << ')' << std::endl;
		
		// 遍历重叠范围内的每个体素单元，记录内部单元
		/*std::cout << "Recording interior cells..." << std::endl;
		vox.voxels.for_each_in_range_unchecked(
			[this, offset](vec3s pos, voxelizer::cell_type type) {
				if (type == voxelizer::cell_type::interior) {
					cells.emplace_back(vec3s(vec3i(pos) + offset));
				}
			},
			min_coord, max_coord
		);
		std::cout << "Completed recording interior cells." << std::endl;*/
		std::cout << std::endl;
	}

	bool obstacle::is_cell_inside(const vec3d& cell) const {
		// 判断指定单元是否在内部
		//if (vox.is_interior(cell)) std::cout << "cell:(" << cell.x << ", " << cell.y << ", " << cell.y << ")" << (vox.is_interior(cell) ? "is interior" : " is not interior") << std::endl;
		return vox.is_interior(cell);
	}
	bool obstacle::is_cell_inside(const vec3s& cell) const {
		return vox.is_interior(cell);
	}

	bool obstacle::is_cell_on_surface(const vec3d& cell) const {
		// 判断指定单元是否在表面
		return vox.is_surface(cell);
	}
	bool obstacle::is_cell_on_surface(const vec3s& cell) const {
		return vox.is_surface(cell);
	}

	bool obstacle::is_cell_outside(const vec3d& cell) const {
		// 判断指定单元是否在外部
		return vox.is_exterior(cell);
	}
	bool obstacle::is_cell_outside(const vec3s& cell) const {
		return vox.is_exterior(cell);
	}
}
