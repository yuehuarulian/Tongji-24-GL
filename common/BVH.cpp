#include <BVH.hpp>
#include <iostream>

// 计算一个三角形的AABB 包围盒
AABB getTriangleAABB(const Vertex &v0, const Vertex &v1, const Vertex &v2)
{
    glm::vec3 minVertex = glm::min(v0.Position, glm::min(v1.Position, v2.Position));
    glm::vec3 maxVertex = glm::max(v0.Position, glm::max(v1.Position, v2.Position));

    return AABB{minVertex, maxVertex};
}

BVHNode::BVHNode(const AABB &bbox, const std::vector<unsigned int> &indices)
{
    this->bbox = bbox;
    this->triangleIndices = indices;
}
BVHNode::BVHNode(const AABB &bbox, BVHNode *left, BVHNode *right)
{
    this->bbox = bbox;
    this->left = left;
    this->right = right;
}

BVHNode *buildBVHTree(const std::vector<unsigned int> &indices, const std::vector<Vertex> &vertices, int BVHDepth)
{
    // 输出当前深度
    std::cout << "Building BVH at depth " << BVHDepth << ", with " << indices.size() << " indices." << std::endl;

    if (indices.size() == 1)
    {
        // 叶子节点
        AABB bbox = getTriangleAABB(vertices[indices[0]], vertices[indices[1]], vertices[indices[2]]);
        // std::cout << "Leaf node: bbox = " << bbox << " for triangle indices (" << indices[0] << ", " << indices[1] << ", " << indices[2] << ")" << std::endl;
        std::cout << "AABB(min: [" << bbox.min.x << ", " << bbox.min.y << ", " << bbox.min.z << "], "
                  << "max: [" << bbox.max.x << ", " << bbox.max.y << ", " << bbox.max.z << "])" << std::endl;
        return new BVHNode(bbox, indices);
    }

    // 计算包围盒
    AABB bbox;
    for (unsigned int idx : indices)
    {
        AABB triangleBbox = getTriangleAABB(vertices[indices[idx]], vertices[indices[idx + 1]], vertices[indices[idx + 2]]);
        bbox = bbox.merge(triangleBbox);
    }

    // 输出包围盒信息
    std::cout << "Bounding box for current node: ";
    std::cout << "AABB(min: [" << bbox.min.x << ", " << bbox.min.y << ", " << bbox.min.z << "], "
              << "max: [" << bbox.max.x << ", " << bbox.max.y << ", " << bbox.max.z << "])" << std::endl;

    // 根据包围盒的中心分割三角形
    std::vector<unsigned int> leftIndices, rightIndices;
    glm::vec3 centroid = (bbox.min + bbox.max) / 2.0f;
    std::cout << "Centroid: " << centroid.x << ", " << centroid.y << ", " << centroid.z << std::endl;

    for (unsigned int idx : indices)
    {
        const Vertex &v0 = vertices[indices[idx]];
        const Vertex &v1 = vertices[indices[idx + 1]];
        const Vertex &v2 = vertices[indices[idx + 2]];
        glm::vec3 center = (v0.Position + v1.Position + v2.Position) / 3.0f;

        // 输出每个三角形的重心
        std::cout << "Triangle center for indices (" << indices[0] << ", " << indices[1] << ", " << indices[2] << "): "
                  << center.x << ", " << center.y << ", " << center.z << std::endl;

        if (center.x < centroid.x)
        {
            leftIndices.push_back(idx);
            std::cout << "Added index " << idx << " to left child." << std::endl;
        }
        else
        {
            rightIndices.push_back(idx);
            std::cout << "Added index " << idx << " to right child." << std::endl;
        }
    }

    if (leftIndices.empty() || rightIndices.empty())
    {
        return new BVHNode(bbox, indices);
    }

    // 递归构建左右子树
    std::cout << "Building left child with " << leftIndices.size() << " indices." << std::endl;
    BVHNode *leftChild = buildBVHTree(leftIndices, vertices, ++BVHDepth);
    std::cout << "Building right child with " << rightIndices.size() << " indices." << std::endl;
    BVHNode *rightChild = buildBVHTree(rightIndices, vertices, ++BVHDepth);

    // 输出当前节点的父子关系
    std::cout << "Creating internal node with left and right children." << std::endl;

    return new BVHNode(bbox, leftChild, rightChild);
}
