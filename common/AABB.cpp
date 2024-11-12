#include <AABB.hpp>

const AABB AABB::InValid = AABB(glm::vec3(FLT_MAX), glm::vec3(-FLT_MAX), false);

AABB::AABB()
{
    *this = InValid;
}

AABB::AABB(const glm::vec3 &minP, const glm::vec3 &maxP, bool isValid)
    : minP(minP), maxP(maxP), isValid(isValid) {}

float AABB::GetSurfaceArea() const
{
    glm::vec3 extent = maxP - minP;
    return 2 * (extent.x * extent.y + extent.x * extent.z + extent.y * extent.z);
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

const AABB AABB::operator+(const AABB &aabb) const
{
    if (isValid)
    {
        if (aabb.isValid)
        {
            glm::vec3 minP = min(this->minP, aabb.minP);
            glm::vec3 maxP = max(this->maxP, aabb.maxP);

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

AABB &AABB::operator+=(const AABB &aabb)
{
    Expand(aabb);

    return *this;
}

bool AABB::intersect(const AABB &other) const
{
    return (maxP.x >= other.minP.x && minP.x <= other.maxP.x) &&
           (maxP.y >= other.minP.y && minP.y <= other.maxP.y) &&
           (maxP.z >= other.minP.z && minP.z <= other.maxP.z);
}

bool AABB::Contains(const glm::vec3 &point) const
{
    return point.x >= minP.x && point.x <= maxP.x &&
           point.y >= minP.y && point.y <= maxP.y &&
           point.z >= minP.z && point.z <= maxP.z;
}

AABB getTriangleAABB(const glm::vec3 &v0, const glm::vec3 &v1, const glm::vec3 &v2)
{
    glm::vec3 minP = glm::min(v0, glm::min(v1, v2));
    glm::vec3 maxP = glm::max(v0, glm::max(v1, v2));

    return AABB(minP, maxP);
}