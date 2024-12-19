#include <AABB.hpp>

// Grow the bounding box by a point
void AABB::grow(glm::vec3 const &p)
{
    minP = glm::min(minP, p);
    maxP = glm::max(maxP, p);
}
// Grow the bounding box by a box
void AABB::grow(AABB const &b)
{
    minP = glm::min(minP, b.getMinP());
    maxP = glm::max(maxP, b.getMaxP());
}

bool AABB::contains(glm::vec3 const &p) const
{
    glm::vec3 radius = getExtent() * 0.5f;
    return std::fabs(getCenter().x - p.x) <= radius.x &&
           std::fabs(getCenter().y - p.y) <= radius.y &&
           std::fabs(getCenter().z - p.z) <= radius.z;
}

AABB getTriangleAABB(const glm::vec3 &v0, const glm::vec3 &v1, const glm::vec3 &v2)
{
    AABB bbox;
    bbox.grow(v0);
    bbox.grow(v1);
    bbox.grow(v2);
    return bbox;
}

AABB bboxunion(AABB const &box1, AABB const &box2)
{
    glm::vec3 minP = glm::min(box1.getMinP(), box2.getMinP());
    glm::vec3 maxP = glm::max(box1.getMaxP(), box2.getMaxP());
    return AABB(minP, maxP);
}

AABB intersection(AABB const &box1, AABB const &box2)
{
    glm::vec3 minP = glm::max(box1.getMinP(), box2.getMinP());
    glm::vec3 maxP = glm::min(box1.getMaxP(), box2.getMaxP());
    return AABB(minP, maxP);
}

// 判断两个包围盒是否相交（即是否有重叠）
#define BBOX_INTERSECTION_EPS 0.f
bool intersects(AABB const &box1, AABB const &box2)
{
    glm::vec3 b1c = box1.getCenter();
    glm::vec3 b1r = box1.getExtent() * 0.5f;
    glm::vec3 b2c = box2.getCenter();
    glm::vec3 b2r = box2.getExtent() * 0.5f;

    return (fabs(b2c.x - b1c.x) - (b1r.x + b2r.x)) <= BBOX_INTERSECTION_EPS &&
           (fabs(b2c.y - b1c.y) - (b1r.y + b2r.y)) <= BBOX_INTERSECTION_EPS &&
           (fabs(b2c.z - b1c.z) - (b1r.z + b2r.z)) <= BBOX_INTERSECTION_EPS;
}

// 判断一个包围盒（box1）是否完全包含另一个包围盒（box2）
bool contains(AABB const &box1, AABB const &box2)
{
    return box1.contains(box2.getMinP()) && box1.contains(box2.getMaxP());
}
