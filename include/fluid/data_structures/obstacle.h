#pragma once

/// \file
/// Definition of static obstacles.

#include "mesh.h"
//#include "fluid/data_structures/space_hashing.h"
#include "fluid/voxelizer.h"

namespace fluid {
	/// 静态障碍物类
	class obstacle {
	public:
		using mesh_t = mesh<double, std::size_t, double, double, vec3d>; ///< 网格类型

		/// 默认构造函数
		obstacle() = default;

		/**
		 * @brief 使用给定的网格体素化来构造障碍物
		 * @param m 障碍物的网格
		 * @param cell_size 网格单元大小
		 * @param ref_grid_offset 参考网格的偏移量
		 * @param ref_grid_size 参考网格的大小
		 */
		obstacle(mesh_t m, double cell_size, vec3d ref_grid_offset, vec3s ref_grid_size);

		/**
		 * @brief 判断指定的单元是否在障碍物内部
		 * @param cell 要判断的单元坐标
		 * @return 如果单元在障碍物内部返回 true，否则返回 false
		 */
		bool is_cell_inside(const vec3d& cell) const;
		bool is_cell_inside(const vec3s& cell) const;
		bool is_cell_on_surface(const vec3d& cell) const;
		bool is_cell_on_surface(const vec3s& cell) const;
		bool is_cell_outside(const vec3d& cell) const;
		bool is_cell_outside(const vec3s& cell) const;

	private:
		voxelizer vox;
		mesh_t obstacle_mesh; ///< 障碍物的网格
		//std::vector<vec3s> cells; ///< 完全被障碍物占据的单元
		//space_hashing<3, bool> cell_set; // 使用 3 维的 space_hashing，存储类型为 bool
	};
}