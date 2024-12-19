#ifndef PRIMITIVE_HPP
#define PROMITIVE_HPP

#include <AABB.hpp>

// 图元基类
class Primitive
{
public:
    virtual ~Primitive() = default;

    // 获取包围盒
    virtual AABB getbbox() const = 0;

    // 计算表面面积
    virtual float surfaceArea() const = 0;

    // 计算体积
    virtual float volume() const = 0;

    // 获取几何体的质心
    virtual glm::vec3 centroid() const = 0;
};

// 三角类
class Triangle : public Primitive
{
public:
    Triangle(const glm::vec3 &v0, const glm::vec3 &v1, const glm::vec3 &v2)
        : v0(v0), v1(v1), v2(v2)
    {
        glm::vec3 minP = glm::min(glm::min(v0, v1), v2);
        glm::vec3 maxP = glm::max(glm::max(v0, v1), v2);
        bbox = AABB(minP, maxP);
    }

    const glm::vec3 &getv0() const { return v0; }
    const glm::vec3 &getv1() const { return v1; }
    const glm::vec3 &getv2() const { return v2; }
    AABB getbbox() const override { return bbox; }

    float surfaceArea() const override
    {
        glm::vec3 e0 = v1 - v0;
        glm::vec3 e1 = v2 - v0;
        return 0.5f * glm::length(glm::cross(e0, e1));
    }

    float volume() const override
    {
        // 三角形没有体积，通常返回 0
        return 0.0f;
    }

    glm::vec3 centroid() const override
    {
        return (v0 + v1 + v2) / 3.0f;
    }

private:
    glm::vec3 v0, v1, v2; // 顶点
    AABB bbox;
};

#endif