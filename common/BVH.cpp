#include <BVH.hpp>
#include <iostream>
#include <algorithm>

// BVH 节点构造函数：叶节点
BVHNode::BVHNode(const AABB &bbox, const std::vector<unsigned int> &indices)
    : bbox(bbox), triangleIndices(indices), left(nullptr), right(nullptr) {}

// BVH 节点构造函数：内部节点
BVHNode::BVHNode(const AABB &bbox, BVHNode *left, BVHNode *right)
    : bbox(bbox), left(left), right(right) {}

BVHNode *buildBVHTree(const std::vector<unsigned int> &indices, const std::vector<Vertex> &vertices, int BVHDepth)
{
    // 打印当前深度和三角形数量
    std::cout << "Building BVH at depth " << BVHDepth << ", with " << indices.size() << " indices." << std::endl;

    // 叶子节点条件
    if (indices.size() <= 3)
    {
        AABB bbox = getTriangleAABB(vertices[indices[0]].Position, vertices[indices[1]].Position, vertices[indices[2]].Position);
        std::cout << bbox << std::endl;
        return new BVHNode(bbox, indices);
    }

    // 计算当前节点的包围盒
    AABB bbox;
    for (size_t i = 0; i < indices.size(); i += 3)
    {
        AABB triangleBbox = getTriangleAABB(vertices[indices[i]].Position, vertices[indices[i + 1]].Position, vertices[indices[i + 2]].Position);
        bbox += triangleBbox;
    }

    // 输出包围盒信息
    std::cout << bbox << std::endl;

    // 选择分割轴：根据当前深度选择 x、y、z 轴中的一个
    int axis = BVHDepth % 3;
    float splitValue = (bbox.GetMinP()[axis] + bbox.GetMaxP()[axis]) / 2.0f;

    // 分割索引到左右子节点
    std::vector<unsigned int> leftIndices, rightIndices;
    for (size_t i = 0; i < indices.size(); i += 3)
    {
        const Vertex &v0 = vertices[indices[i]];
        const Vertex &v1 = vertices[indices[i + 1]];
        const Vertex &v2 = vertices[indices[i + 2]];
        glm::vec3 center = (v0.Position + v1.Position + v2.Position) / 3.0f;

        // 根据分割轴分配到左右子节点
        if (center[axis] < splitValue)
        {
            leftIndices.push_back(indices[i]);
            leftIndices.push_back(indices[i + 1]);
            leftIndices.push_back(indices[i + 2]);
        }
        else
        {
            rightIndices.push_back(indices[i]);
            rightIndices.push_back(indices[i + 1]);
            rightIndices.push_back(indices[i + 2]);
        }
    }

    // 如果分割失败，直接创建叶节点
    if (leftIndices.empty() || rightIndices.empty())
    {
        std::cout << "Failed to split; creating leaf node." << std::endl;
        return new BVHNode(bbox, indices);
    }

    // 递归构建左右子树
    std::cout << "Building left child with " << leftIndices.size() / 3 << " triangles." << std::endl;
    BVHNode *leftChild = buildBVHTree(leftIndices, vertices, BVHDepth + 1);

    std::cout << "Building right child with " << rightIndices.size() / 3 << " triangles." << std::endl;
    BVHNode *rightChild = buildBVHTree(rightIndices, vertices, BVHDepth + 1);

    // 创建内部节点
    return new BVHNode(bbox, leftChild, rightChild);
}
