#ifndef _RAG_MATH_H_
#define _RAY_MATH_H_

#include <glm/glm.hpp>
#include <vector>

namespace Basic
{
    namespace Math
    {
        const float EPSILON = 10e-6f;
        const float PI = 3.1415926f;

        // 转换为灰度值
        // 计算公式：0.299*R+0.587*G+0.114*B
        float Gray(const glm::vec3 &color);

        // 计算光线的折射方向
        // 1. viewDir: 入射光的方向
        // 2. normal： 表面法线
        // 3. ratioNiNt: 折射率之比 = Ni / Nt
        //   Ni 为 入射光线所在介质的折射率
        //   Nt 为 折射光线所在介质的折射率
        // 4. refracDir: 输出参数 -- 存储计算出的折射光线方向
        bool ReFract(const glm::vec3 &viewDir, const glm::vec3 &normal, float ratioNiNt, glm::vec3 &refractDir);

        // 计算 Fresnel 方程 中的反射率，使用的是 Schlick 的近似公式
        float FresnelSchlick(const glm::vec3 &viewDir, const glm::vec3 &halfway, float ratioNiNt);

        // 计算平均值
        template <typename T>
        T Mean(const std::vector<T> &data);

        // 计算方差
        template <typename T>
        T Variance(const std::vector<T> &data);

        // 计算最小值
        template <typename T>
        T min(const std::vector<T> &val);

        // 计算最大值
        template <typename T>
        T max(const std::vector<T> &val);

        // 将一个 三维单位向量（通常是法线向量）映射到一个 二维纹理坐标（UV坐标）
        glm::vec2 Sphere2UV(const glm::vec3 &normal);

        // 计算 射线与三角形的相交。该函数使用的是 Möller–Trumbore 算法
        glm::vec4 Intersect_RayTri(const glm::vec3 &e, const glm::vec3 &d, const glm::vec3 &a, const glm::vec3 &b, const glm::vec3 &c);
    }
}

#endif