#pragma once

/// \file
/// Definition of static obstacles.

#include "mesh.h"
//#include "fluid/data_structures/space_hashing.h"
#include "fluid/voxelizer.h"

namespace fluid {
	/// ��̬�ϰ�����
	class obstacle {
	public:
		using mesh_t = mesh<double, std::size_t, double, double, vec3d>; ///< ��������

		/// Ĭ�Ϲ��캯��
		obstacle() = default;

		/**
		 * @brief ʹ�ø������������ػ��������ϰ���
		 * @param m �ϰ��������
		 * @param cell_size ����Ԫ��С
		 * @param ref_grid_offset �ο������ƫ����
		 * @param ref_grid_size �ο�����Ĵ�С
		 */
		obstacle(mesh_t m, double cell_size, vec3d ref_grid_offset, vec3s ref_grid_size);

		/**
		 * @brief �ж�ָ���ĵ�Ԫ�Ƿ����ϰ����ڲ�
		 * @param cell Ҫ�жϵĵ�Ԫ����
		 * @return �����Ԫ���ϰ����ڲ����� true�����򷵻� false
		 */
		bool is_cell_inside(const vec3d& cell) const;
		bool is_cell_inside(const vec3s& cell) const;
		bool is_cell_on_surface(const vec3d& cell) const;
		bool is_cell_on_surface(const vec3s& cell) const;
		bool is_cell_outside(const vec3d& cell) const;
		bool is_cell_outside(const vec3s& cell) const;

	private:
		voxelizer vox;
		mesh_t obstacle_mesh; ///< �ϰ��������
		//std::vector<vec3s> cells; ///< ��ȫ���ϰ���ռ�ݵĵ�Ԫ
		//space_hashing<3, bool> cell_set; // ʹ�� 3 ά�� space_hashing���洢����Ϊ bool
	};
}