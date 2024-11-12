#ifndef BVH_HPP
#define BVH_HPP

#include <glm/glm.hpp>
#include <vector>
#include <iostream>
#define MAX_BONE_INFLUENCE 4

struct Vertex
{
    // position
    glm::vec3 Position;
    // normal
    glm::vec3 Normal;
    // texCoords
    glm::vec2 TexCoords;
    // tangent
    glm::vec3 Tangent;
    // bitangent
    glm::vec3 Bitangent;
    // bone indexes which will influence this vertex
    int m_BoneIDs[MAX_BONE_INFLUENCE];
    // weights from each bone
    float m_Weights[MAX_BONE_INFLUENCE];
};

// 包围盒
struct AABB
{
    glm::vec3 min; // 最小点
    glm::vec3 max; // 最大点

    // 合并两个 AABB，返回包含这两个 AABB 的最小包围盒
    AABB merge(const AABB &other) const
    {
        return AABB{
            glm::min(min, other.min),
            glm::max(max, other.max)};
    }

    // 判断两者是否相交 -- 即是否右重叠位置
    bool intersect(const AABB &other) const
    {
        return (max.x >= other.min.x && min.x <= other.max.x) &&
               (max.y >= other.min.y && min.y <= other.max.y) &&
               (max.z >= other.min.z && min.z <= other.max.z);
    }
};

// // 重载<<运算符来打印AABB对象
// std::ostream &operator<<(std::ostream &os, const AABB &bbox)
// {
//     os << "AABB(min: [" << bbox.min.x << ", " << bbox.min.y << ", " << bbox.min.z << "], "
//        << "max: [" << bbox.max.x << ", " << bbox.max.y << ", " << bbox.max.z << "])";
//     return os;
// }

class BVHNode
{
public:
    AABB bbox;                                 // 当前节点的包围盒
    BVHNode *left = nullptr;                   // 左孩子节点
    BVHNode *right = nullptr;                  // 右孩子节点
    std::vector<unsigned int> triangleIndices; // 存储三角形的索引

    BVHNode(const AABB &bbox, const std::vector<unsigned int> &indices); // 构造叶子节点
    BVHNode(const AABB &bbox, BVHNode *left, BVHNode *right);            // 构造内部节点
};

AABB getTriangleAABB(const Vertex &v0, const Vertex &v1, const Vertex &v2);

BVHNode *buildBVHTree(const std::vector<unsigned int> &indices, const std::vector<Vertex> &vertices, int BVHDepth);

#endif