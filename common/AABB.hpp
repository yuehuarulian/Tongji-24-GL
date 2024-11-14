#ifndef AABB_HPP
#define AABB_HPP

#include <glm/glm.hpp>
#include <iostream>

class AABB
{
public:
    AABB()
        : minP(glm::vec3(std::numeric_limits<float>::max(),
                         std::numeric_limits<float>::max(),
                         std::numeric_limits<float>::max())),
          maxP(glm::vec3(-std::numeric_limits<float>::max(),
                         -std::numeric_limits<float>::max(),
                         -std::numeric_limits<float>::max())) {}
    AABB(glm::vec3 const &p)
        : minP(p), maxP(p) {}
    AABB(glm::vec3 const &p1, glm::vec3 const &p2)
        : minP(glm::min(p1, p2)), maxP(glm::max(p1, p2)) {}

    // gets函数
    inline glm::vec3 getMinP() const { return minP; }
    inline glm::vec3 getMaxP() const { return maxP; }
    glm::vec3 getCenter() const { return (maxP + minP) * 0.5f; }
    glm::vec3 getExtent() const { return maxP - minP; }
    float getRadius() const { return glm::length(maxP - minP) / 2.0f; }
    float getSurfaceArea() const
    {
        glm::vec3 ext = getExtent();
        return 2.f * (ext.x * ext.y + ext.x * ext.z + ext.y * ext.z);
    }

    bool contains(glm::vec3 const &p) const;

    void grow(glm::vec3 const &p); // Grow the bounding box by a point
    void grow(AABB const &b);      // Grow the bounding box by a box

    inline int maxdim() const
    {
        glm::vec3 ext = getExtent();

        if (ext.x >= ext.y && ext.x >= ext.z)
            return 0;
        if (ext.y >= ext.x && ext.y >= ext.z)
            return 1;
        if (ext.z >= ext.x && ext.z >= ext.y)
            return 2;

        return 0;
    }
    glm::vec3 const &operator[](int i) const { return *(&minP + i); }

    // 重载输出运算符
    friend std::ostream &operator<<(std::ostream &os, const AABB &aabb)
    {
        os << "AABB { "
           << "Min: (" << aabb.getMinP().x << ", " << aabb.getMinP().y << ", " << aabb.getMinP().z << "), "
           << "Max: (" << aabb.getMaxP().x << ", " << aabb.getMaxP().y << ", " << aabb.getMaxP().z << "), "
           << " }";
        return os;
    }

private:
    glm::vec3 minP; // 最小点
    glm::vec3 maxP; // 最大点
};

AABB getTriangleAABB(const glm::vec3 &v0, const glm::vec3 &v1, const glm::vec3 &v2);
AABB bboxunion(AABB const &box1, AABB const &box2);
AABB intersection(AABB const &box1, AABB const &box2);
bool intersects(AABB const &box1, AABB const &box2);
bool contains(AABB const &box1, AABB const &box2);

#endif