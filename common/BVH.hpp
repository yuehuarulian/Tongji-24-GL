#ifndef BVH_HPP
#define BVH_HPP

#include <glm/glm.hpp>
#include <vector>
#include <iostream>
#include <AABB.hpp>
#include <Primitive.hpp>

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

class BVHNode
{
public:
    AABB bbox;                                 // 当前节点的包围盒
    BVHNode *left = nullptr;                   // 左孩子节点
    BVHNode *right = nullptr;                  // 右孩子节点
    bool isLeaf;                               // 是否为叶子节点
    int start, end;                            // 叶子节点的 primitive 范围
    std::vector<unsigned int> triangleIndices; // 存储三角形的索引

    BVHNode(const AABB &bbox, const std::vector<unsigned int> &indices); // 构造叶子节点
    BVHNode(const AABB &bbox, BVHNode *left, BVHNode *right);            // 构造内部节点
};

BVHNode *buildBVHTree(const std::vector<unsigned int> &indices, const std::vector<Vertex> &vertices, int BVHDepth);

class BVHManager
{
public:
private:
    int nBuckets;                        // 分割桶数
    int maxPrimsInNode;                  // 每个节点最多的 Primitive 数
    std::vector<Primitive *> primitives; // 存储 Primitive 的数组
    struct Bucket
    {
        int count;
        AABB bbox;
    };
    std::vector<Bucket> buckets; // 存储桶数据
};

#endif