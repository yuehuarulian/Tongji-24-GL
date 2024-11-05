#ifndef _AABB_H_
#define _AABB_H_

#include <Basic/Ptr.hpp>
#include <glm/glm.hpp>
/*
    AABB(Axis-aligned bounding box) 轴对齐包围盒 -- 用于检测物体碰撞
*/

namespace RTX
{
    class Ray; // 光线类
    class AABB
    {
    public:
        AABB();
        AABB(const glm::vec3 &minP, const glm::vec3 &maxP, bool isValid = true);

        inline glm::vec3 GetMinP() const { return minP; }
        inline glm::vec3 GetMaxP() const { return maxP; }
        inline glm::vec3 GetExtent() const { return maxP - minP; }
        inline glm::vec3 GetCenter() const { return (minP + maxP) / 2.0f; }
        inline float GetRadius() const { return length(maxP - minP) / 2.0f; }
        float GetSurfaceArea() const;

        void SetP(const glm::vec3 &minP, const glm::vec3 &maxP);
        inline bool IsValid() const { return isValid; }
        bool Hit(Basic::Ptr<Ray> ray) const;
        bool Hit(Basic::Ptr<Ray> ray, float &tMin, float &tMax) const;

        void Expand(const AABB &aabb);

        const AABB operator+(const AABB &aabb) const;
        AABB &operator+=(const AABB &aabb);

        static const AABB InValid;

    private:
        bool isValid;
        glm::vec3 minP;
        glm::vec3 maxP;
    };
}

#endif