#ifndef BVH_HPP
#define BVH_HPP

#include <glm/glm.hpp>
#include <vector>
#include <atomic>
#include <cmath>
#include <numeric>
#include <iostream>
#include <AABB.hpp>

#define MAX_BONE_INFLUENCE 4

class BVH
{
public:
    BVH(float traversal_cost, int num_bins = 64, bool usesah = true)
        : m_root(nullptr),
          m_num_bins(num_bins),
          m_usesah(usesah),
          m_height(0),
          m_traversal_cost(traversal_cost) {}
    ~BVH() = default;

    AABB const &getBounds() const { return m_bounds; }
    inline int getHeight() const { return m_height; }
    virtual int const *getIndices() const { return &m_packed_indices[0]; }
    virtual size_t getNumIndices() const { return m_packed_indices.size(); }

    void Build(AABB const *bounds, int numbounds);

    // 打印BVH树数据
    virtual void PrintStatistics(std::ostream &os) const;

protected:
    virtual void BuildImpl(AABB const *bounds, int numbounds);

    enum NodeType
    {
        kInternal,
        kLeaf
    };
    struct Node
    {
        AABB bounds;
        NodeType type;
        int index;
        union
        {
            // internal nodes
            struct
            {
                Node *leftChild;
                Node *rightChild;
            };
            // leaf nodes
            struct
            {
                int startidx;
                int numprims;
            };
        };
    };

    virtual Node *AllocateNode();
    virtual void InitNodeAllocator(size_t maxnum);
    // 记录分割请求信息
    struct SplitRequest
    {
        int startidx;         // 当前分割请求的起始索引
        int numprims;         // 当前分割请求中包含的物体数量
        Node **ptr;           // 当前分割请求的根节点指针，用于存储分割后的左右子树节点
        AABB bounds;          // 当前分割请求的边界框，包含该请求处理的所有物体的边界
        AABB centroid_bounds; // 当前分割请求中物体的质心边界框
        int level;            // 当前分割请求的深度级别
        int index;            // 当前分割请求中节点的索引
    };
    // 描述使用 SAH（Surface Area Heuristic）算法计算的分割信息
    struct SahSplit
    {
        int dim;       // 分割的维度
        float split;   // 分割位置
        float sah;     // SAH值，表示该分割方案的质量，值越小越好，表示分割后左右子树的代价最小
        float overlap; // 重叠度，表示分割后左右子树的重叠程度。重叠度较低通常表示分割质量较好
    };

    SahSplit FindSahSplit(SplitRequest const &req, AABB const *bounds, glm::vec3 const *centroids, int *primindices) const;
    void BuildNode(SplitRequest const &req, AABB const *bounds, glm::vec3 const *centroids, int *primindices);

    std::vector<Node> m_nodes;  // BVH Nodes
    std::vector<int> m_indices; // 索引
    std::atomic<int> m_nodecnt; // Number of Nodes
    std::vector<int> m_packed_indices;
    AABB m_bounds;
    Node *m_root;
    int m_height;  // 树的高度
    bool m_usesah; // 是否使用SAH（Surface Area Heuristic）算法进行分割
    float m_traversal_cost;
    int m_num_bins;

private:
    BVH(BVH const &) = delete;
    BVH &operator=(BVH const &) = delete;
};

#endif