#include "fluid/renderer/camera.h"

/// \file
/// Implementation of the camera.

#include "fluid/renderer/common.h"

namespace fluid::renderer {
	// �Ӳ�������һ���������
	camera camera::from_parameters(vec3d pos, vec3d ref, vec3d up, double fovy_radians, double width_over_height) {
		camera result;  // ����һ���յ��������
		result.position = pos;  // ���������λ��Ϊ�����е� pos
		result.norm_forward = (ref - pos).normalized_unchecked();  // ��������ĳ�ǰ���򣬲���һ��

		// �����ӳ��ǵ�һ�������ֵ
		double tan_half = std::tan(0.5 * fovy_radians);

		// ����������ҷ������� (half_horizontal)��ʹ��ǰ��������Ϸ���Ĳ���õ��ҷ�������������һ��
		result.half_horizontal = vec_ops::cross(result.norm_forward, up)
			.normalized_checked()  // �����һ��ʧ�ܣ����������������ṩĬ�ϵ��ҷ�������
			.value_or(get_cross_product_axis(result.norm_forward));
		
		// ����������Ϸ������� (half_vertical)��ʹ��ǰ������ҷ���Ĳ���õ��Ϸ�������
		result.half_vertical = vec_ops::cross(result.norm_forward, result.half_horizontal);

		// �����ҷ��������Ĵ�С��ƥ���ӳ����
		result.half_horizontal *= tan_half * width_over_height;
		
		// �����Ϸ��������Ĵ�С��ƥ���ӳ��߶�
		result.half_vertical *= tan_half;

		return result;  // �������úõ��������
	}

	// ������Ļ�����ȡһ������
	ray camera::get_ray(vec2d screen_pos) const {
		// ����Ļ����� [0, 1] ��Χӳ�䵽 [-1, 1]
		screen_pos = screen_pos * 2.0 - vec2d(1.0, 1.0);

		ray result;  // ����һ�����߶���
		result.origin = position;  // �������ߵ����Ϊ�����λ��

		// �������ߵķ���ʹ�������ǰ�����ҷ��򣨸���ˮƽ�����Ȩ�����Ϸ��򣨸��ݴ�ֱ�����Ȩ�����ӵõ�
		result.direction = norm_forward + screen_pos.x * half_horizontal + screen_pos.y * half_vertical;
		
		return result;  // ���ؼ���õ����߶���
	}
}
