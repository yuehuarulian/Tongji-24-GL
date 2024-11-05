#include <RTX/AABB.hpp>
#include <RTX/Ray.hpp>

using namespace RTX;
using namespace glm;

const AABB AABB::InValid(vec3(1), vec3(-1), false);

AABB::AABB()
{
    *this = InValid;
}

AABB::AABB(const vec3 &minP, const vec3 &maxP, bool isValid)
    : minP(minP), maxP(maxP), isValid(isValid) {}

float AABB::GetSurfaceArea() const
{
    // 计算并返回轴对齐包围盒的表面积
    vec3 extent = maxP - minP;
    return 2 * (extent.x * extent.y + extent.x * extent.z + extent.y * extent.z);
}

void AABB::SetP(const glm::vec3 &minP, const glm::vec3 &maxP)
{
    this->minP = minP;
    this->maxP = maxP;
    isValid = true;
}

bool AABB::Hit(Ray::Ptr ray, float &tMin, float &tMax) const
{
    // 判断光线是否以该轴对齐包围盒相交
    // 同时返回相交的最近时间和最远时间
    if (!IsValid())
        return false;

    const vec3 origin = ray->GetOrigin();
    const vec3 dir = ray->GetDir();
    tMin = Ray::tMin;
    tMax = ray->GetTMax();

    for (size_t i = 0; i < 3; i++)
    {
        float invD = 1.0f / dir[i];
        float t0 = (minP[i] - origin[i]) * invD; // 计算进入 AABB 的 t 值
        float t1 = (maxP[i] - origin[i]) * invD; // 计算退出 AABB 的 t 值
        if (invD < 0.0f)
            std::swap(t0, t1);

        tMin = max(t0, tMin);
        tMax = min(t1, tMax);
        if (tMax <= tMin)
            return false;
    }
    return true;
}

bool AABB::Hit(Ray::Ptr ray) const
{
    // 仅判断光线是否与物体相交
    float tMin, tMax;
    return Hit(ray, tMin, tMax);
}

const AABB AABB::operator+(const AABB &aabb) const
{
    // 两个轴对齐包围盒相加 -- 返回一个大的轴对齐包围盒
    if (isValid)
    {
        if (aabb.isValid)
        {
            vec3 minP = min(this->minP, aabb.minP);
            vec3 maxP = max(this->maxP, aabb.maxP);

            return {minP, maxP};
        }
        else
            return *this;
    }
    else
    {
        if (aabb.isValid)
            return aabb;
        else
            return AABB::InValid;
    }
}

void AABB::Expand(const AABB &aabb)
{
    if (aabb.isValid)
    {
        if (isValid)
        {
            minP = min(minP, aabb.minP);
            maxP = max(maxP, aabb.maxP);
        }
        else
        {
            minP = aabb.minP;
            maxP = aabb.maxP;
            isValid = true;
        }
    }
}

AABB &AABB::operator+=(const AABB &aabb)
{
    Expand(aabb);

    return *this;
}