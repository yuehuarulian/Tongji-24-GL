#include "fluid/voxelizer.h"

/// \file
/// Implementation of the voxelizer.

#include <algorithm>
#include <stack>
#include <iostream>//debug

#include "fluid/math/intersection.h"

namespace fluid {
	// ������Ĵ�С��λ�õ���Ϊ����ָ���ı߽��
	void voxelizer::resize_reposition_grid(vec3d min, vec3d max) {
		vec3d
			size = max - min, // ����߽��ĳߴ�
			// ���������С����Ӧ������ȡ������
			grid_size = vec_ops::apply<vec3d>(static_cast<double (*)(double)>(std::ceil), size / cell_size);
		// ���������ƫ������ȷ���������߽��
		grid_offset = min - 0.5 * (grid_size * cell_size - size) - vec3d(cell_size, cell_size, cell_size);
		// ��ʼ�����������С������ÿ�����ر��Ϊ�ڲ�����
		voxels = grid3<cell_type>(vec3s(grid_size) + vec3s(2, 2, 2), cell_type::interior);
	}

	// Լ�����������С��λ�ã��ο�ָ���ĵ�Ԫ��С������ƫ����
	vec3i voxelizer::resize_reposition_grid_constrained(
		vec3d min, vec3d max, double ref_cell_size, vec3d ref_grid_offset
	) {
		cell_size = ref_cell_size; // ���õ�Ԫ��С
		// ������С�����������������Ӧ�����»�����ȡ��
		vec3i
			grid_min(vec_ops::apply<vec3d>(
				static_cast<double (*)(double)>(std::floor), (min - ref_grid_offset) / cell_size)
			),
			grid_max(vec_ops::apply<vec3d>(
				static_cast<double (*)(double)>(std::ceil), (max - ref_grid_offset) / cell_size)
			);
		grid_min -= vec3i(1, 1, 1); // ���ӱ߽�
		grid_max += vec3i(1, 1, 1);
		// ��������ƫ����
		grid_offset = ref_grid_offset + vec3d(grid_min) * cell_size;
		vec3d grid_maxset = ref_grid_offset + vec3d(grid_max) * cell_size;
		std::cout << "  voxelizer��grid_min(" << grid_offset.x << ',' << grid_offset.y << ',' << grid_offset.z << ')' << std::endl;
		std::cout << "  voxelizer��grid_max(" << grid_maxset.x << ',' << grid_maxset.y << ',' << grid_maxset.z << ')' << std::endl;
		std::cout << "  voxelizer��size    (" << vec3s(grid_max - grid_min).x << ',' << vec3s(grid_max - grid_min).y << ',' << vec3s(grid_max - grid_min).z << ')' << std::endl;
		// ��ʼ���������񣬲���ÿ�����ر��Ϊ�ڲ�����
		voxels = grid3<cell_type>(vec3s(grid_max - grid_min), cell_type::interior);
		return grid_min;
	}

	// ��ȡ��ο������ص������ط�Χ
	std::pair<vec3s, vec3s> voxelizer::get_overlapping_cell_range(vec3i offset, vec3s ref_grid_size) const {
		// ������С�ص�����
		vec3s min_coord = vec_ops::apply<vec3s>(
			[](int coord) {
				return coord < 0 ? static_cast<std::size_t>(-coord) : 0;
			},
			offset
				);
		// ��������ص����꣬ȷ���ڲο�����Χ��
		vec3s max_coord = vec_ops::apply<vec3s>(
			[](int coord, std::size_t max) {
				return std::min(static_cast<std::size_t>(std::max(0, coord)), max);
			},
			offset + vec3i(voxels.get_size()), ref_grid_size
				);
		return { min_coord, max_coord };
	}

	// �������� p1, p2, p3 ���ػ�������������
	void voxelizer::voxelize_triangle(vec3d p1, vec3d p2, vec3d p3) {
		vec3d min = p1, max = p1;
		// ���±߽��
		_update_bounding_box(p2, min, max);
		_update_bounding_box(p3, min, max);

		double half_cell_size = 0.5 * cell_size;
		vec3d half_extents = vec3d(half_cell_size, half_cell_size, half_cell_size);

		// here we assume the indices are in range
		// otherwise converting a negative float to unsigned is undefined behavior
		// �������η�Χת��Ϊ����������Χ
		vec3s min_id((min - grid_offset) / cell_size), max_id((max - grid_offset) / cell_size);
		// ������С�������ĵ�
		vec3d min_center = grid_offset + vec3d(min_id) * cell_size + half_extents, center = min_center;
		for (std::size_t z = min_id.z; z <= max_id.z; ++z, center.z += cell_size) {
			center.y = min_center.y;
			for (std::size_t y = min_id.y; y <= max_id.y; ++y, center.y += cell_size) {
				center.x = min_center.x;
				for (std::size_t x = min_id.x; x <= max_id.x; ++x, center.x += cell_size) {
					// ��������Ƿ��ѱ��Ϊ����
					cell_type &type = voxels(x, y, z);
					if (type != cell_type::surface) {
						// ������غ��������ཻ������Ϊ��������
						if (aab_triangle_overlap_bounded(center, half_extents, p1, p2, p3)) {
							type = cell_type::surface;
							//std::cout << "surface voxel(" << x << ',' << y << ',' << z << ')' << std::endl;
						}
					}
				}
			}
		}
	}

	// ����ⲿ����
	void voxelizer::mark_exterior() {
		// �����������Ϊ�գ��򷵻�
		if (voxels.get_array_size(voxels.get_size()) == 0) {
			return;
		}

		// ������½�����Ϊ���棬�򷵻�
		if (voxels(0, 0, 0) == cell_type::surface) {
			return;
		}
		// �����½����ر��Ϊ�ⲿ
		voxels(0, 0, 0) = cell_type::exterior;
		std::stack<vec3s> stack;
		stack.emplace(vec3s(0, 0, 0));

		// ����������ز��������ջ��
		auto check_push = [this, &stack](vec3s p) {
			cell_type &ct = voxels(p);
			if (ct == cell_type::interior) {
				ct = cell_type::exterior;
				stack.emplace(p);
			}
		};

		// ʹ��������ȱ�������������ⲿ����
		while (!stack.empty()) {
			vec3s cur = stack.top();
			stack.pop();

			// ���ÿ�����ڷ��򲢳��Ա��
			if (cur.z > 0) {
				check_push(vec3s(cur.x, cur.y, cur.z - 1));
			}
			if (cur.y > 0) {
				check_push(vec3s(cur.x, cur.y - 1, cur.z));
			}
			if (cur.x > 0) {
				check_push(vec3s(cur.x - 1, cur.y, cur.z));
			}

			if (cur.z + 1 < voxels.get_size().z) {
				check_push(vec3s(cur.x, cur.y, cur.z + 1));
			}
			if (cur.y + 1 < voxels.get_size().y) {
				check_push(vec3s(cur.x, cur.y + 1, cur.z));
			}
			if (cur.x + 1 < voxels.get_size().x) {
				check_push(vec3s(cur.x + 1, cur.y, cur.z));
			}
		}
	}

	// ���������εİ�Χ��
	void voxelizer::_update_bounding_box(vec3d pos, vec3d &min, vec3d &max) {
		vec_ops::for_each(
			[](double v, double &minv, double &maxv) {
				minv = std::min(minv, v);
				maxv = std::max(maxv, v);
			},
			pos, min, max
				);
	}

	// �ж�ָ������������Ƿ�Ϊ�ڲ�����
	bool voxelizer::is_interior(const vec3d& pos) const {
		vec3d p = ((pos - grid_offset) / cell_size);
		// ȷ����������������Χ��
		if (p.x < 0 || p.y < 0 || p.z < 0 ||
			p.x >= voxels.get_size().x || p.y >= voxels.get_size().y || p.z >= voxels.get_size().z)
			 {
			return false; // ���������Χ������ false
		}
		return voxels(vec3s(p)) == cell_type::interior; // ���������Ƿ�Ϊ�ڲ�����
	}
	bool voxelizer::is_interior(const vec3s& pos) const {
		// ȷ����������������Χ��
		if (pos.x < 0 || pos.y < 0 || pos.z < 0 ||
			pos.x >= voxels.get_size().x || pos.y >= voxels.get_size().y || pos.z >= voxels.get_size().z)
		{
			return false; // ���������Χ������ false
		}
		return voxels(pos) == cell_type::interior; // ���������Ƿ�Ϊ�ڲ�����
	}
	// �ж�ָ������������Ƿ�Ϊ��������
	bool voxelizer::is_surface(const vec3d& pos) const {
		vec3d p = ((pos - grid_offset) / cell_size);
		// ȷ����������������Χ��
		if (p.x < 0 || p.y < 0 || p.z < 0 ||
			p.x >= voxels.get_size().x || p.y >= voxels.get_size().y || p.z >= voxels.get_size().z)
		{
			return false; // ���������Χ������ false
		}
		return voxels(vec3s(p)) == cell_type::surface; // ���������Ƿ�Ϊ�ڲ�����
	}
	bool voxelizer::is_surface(const vec3s& pos) const {
		// ȷ����������������Χ��
		if (pos.x < 0 || pos.y < 0 || pos.z < 0 ||
			pos.x >= voxels.get_size().x || pos.y >= voxels.get_size().y || pos.z >= voxels.get_size().z)
		{
			return false; // ���������Χ������ false
		}
		return voxels(pos) == cell_type::surface; // ���������Ƿ�Ϊ�ڲ�����
	}
	// �ж�ָ������������Ƿ�Ϊ�ⲿ����
	bool voxelizer::is_exterior(const vec3d& pos) const {
		vec3d p = ((pos - grid_offset) / cell_size);
		// ȷ����������������Χ��
		if (p.x < 0 || p.y < 0 || p.z < 0 ||
			p.x >= voxels.get_size().x || p.y >= voxels.get_size().y || p.z >= voxels.get_size().z)
		{
			return true; // ���������Χ��Ĭ�Ϸ��� true
		}
		return voxels(vec3s(p)) == cell_type::exterior; // ���������Ƿ�Ϊ�ڲ�����
	}
	bool voxelizer::is_exterior(const vec3s& pos) const {
		// ȷ����������������Χ��
		if (pos.x < 0 || pos.y < 0 || pos.z < 0 ||
			pos.x >= voxels.get_size().x || pos.y >= voxels.get_size().y || pos.z >= voxels.get_size().z)
		{
			return true; // ���������Χ��Ĭ�Ϸ��� true
		}
		return voxels(pos) == cell_type::exterior; // ���������Ƿ�Ϊ�ڲ�����
	}
}
