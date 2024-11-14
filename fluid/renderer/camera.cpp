#include "fluid/renderer/camera.h"

/// \file
/// Implementation of the camera.

#include "fluid/renderer/common.h"

namespace fluid::renderer {
	// 从参数创建一个相机对象
	camera camera::from_parameters(vec3d pos, vec3d ref, vec3d up, double fovy_radians, double width_over_height) {
		camera result;  // 创建一个空的相机对象
		result.position = pos;  // 设置相机的位置为参数中的 pos
		result.norm_forward = (ref - pos).normalized_unchecked();  // 计算相机的朝前方向，并归一化

		// 计算视场角的一半的正切值
		double tan_half = std::tan(0.5 * fovy_radians);

		// 计算相机的右方向向量 (half_horizontal)，使用前方向和向上方向的叉积得到右方向向量，并归一化
		result.half_horizontal = vec_ops::cross(result.norm_forward, up)
			.normalized_checked()  // 如果归一化失败（例如零向量），提供默认的右方向向量
			.value_or(get_cross_product_axis(result.norm_forward));
		
		// 计算相机的上方向向量 (half_vertical)，使用前方向和右方向的叉积得到上方向向量
		result.half_vertical = vec_ops::cross(result.norm_forward, result.half_horizontal);

		// 调整右方向向量的大小以匹配视场宽度
		result.half_horizontal *= tan_half * width_over_height;
		
		// 调整上方向向量的大小以匹配视场高度
		result.half_vertical *= tan_half;

		return result;  // 返回配置好的相机对象
	}

	// 根据屏幕坐标获取一个射线
	ray camera::get_ray(vec2d screen_pos) const {
		// 将屏幕坐标从 [0, 1] 范围映射到 [-1, 1]
		screen_pos = screen_pos * 2.0 - vec2d(1.0, 1.0);

		ray result;  // 创建一个射线对象
		result.origin = position;  // 设置射线的起点为相机的位置

		// 计算射线的方向：使用相机的前方向、右方向（根据水平坐标加权）和上方向（根据垂直坐标加权）叠加得到
		result.direction = norm_forward + screen_pos.x * half_horizontal + screen_pos.y * half_vertical;
		
		return result;  // 返回计算好的射线对象
	}
}
