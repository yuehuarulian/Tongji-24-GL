#ifndef AABB_HPP
#define AABB_HPP

#include <glm/glm.hpp>
#include <iostream>

class AABB
{
public:
    AABB();
    AABB(const glm::vec3 &minP, const glm::vec3 &maxP, bool isValid = true);

    // 获取最小点和最大点
    inline glm::vec3 GetMinP() const { return minP; }
    inline glm::vec3 GetMaxP() const { return maxP; }

    // 判断当前 AABB 是否有效
    inline bool IsValid() const { return isValid; }

    // 获取包围盒的范围（宽度、高度、深度）
    inline glm::vec3 GetExtent() const { return maxP - minP; }

    // 获取包围盒的中心
    inline glm::vec3 GetCenter() const { return (minP + maxP) / 2.0f; }

    // 获取包围盒的半径
    inline float GetRadius() const { return glm::length(maxP - minP) / 2.0f; }

    // 获取包围盒的表面积
    float GetSurfaceArea() const;

    void Expand(const AABB &aabb);                // 扩展包围盒以包含另一个 AABB
    const AABB operator+(const AABB &aabb) const; // 合并两个 AABB 返回新的 AABB
    AABB &operator+=(const AABB &aabb);           // 将当前 AABB 扩展为包含另一个 AABB

    // 检查两个 AABB 是否相交
    bool intersect(const AABB &other) const;

    // 判断 AABB 是否包含一个点
    bool Contains(const glm::vec3 &point) const;

    static const AABB InValid;

    // 重载输出运算符
    friend std::ostream &operator<<(std::ostream &os, const AABB &aabb)
    {
        os << "AABB { "
           << "Min: (" << aabb.GetMinP().x << ", " << aabb.GetMinP().y << ", " << aabb.GetMinP().z << "), "
           << "Max: (" << aabb.GetMaxP().x << ", " << aabb.GetMaxP().y << ", " << aabb.GetMaxP().z << "), "
           << "Valid: " << (aabb.IsValid() ? "True" : "False")
           << " }";
        return os;
    }

private:
    bool isValid;
    glm::vec3 minP; // 最小点
    glm::vec3 maxP; // 最大点
};

AABB getTriangleAABB(const glm::vec3 &v0, const glm::vec3 &v1, const glm::vec3 &v2);

#endif