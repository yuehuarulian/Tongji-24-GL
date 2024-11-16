#include <BVH.hpp>
#include <mesh.hpp>
#include <glm/glm.hpp>

// 用于将基于指针的 BVH（加速结构）表示转换为基于索引的表示
class BVHConverter
{
public:
    BVHConverter() = default;
    struct Node
    {
        glm::vec3 bboxmin;
        glm::vec3 bboxmax;
        glm::vec3 LRLeaf;
    };

    void Process(const BVH *topLevelBvh, const std::vector<Mesh *> &sceneMeshes, const std::vector<MeshInstance> &sceneInstances);
    void ProcessBLAS();
    void ProcessTLAS();
    int topLevelIndex = 0;   // 顶层开始节点
    std::vector<Node> nodes; // 前面储存的是底层 后面存储的是顶层

private:
    int ProcessBLASNodes(const BVH::Node *node);
    int ProcessTLASNodes(const BVH::Node *node);

    int curNode = 0;
    int curTriIndex = 0;
    std::vector<int> bvhRootStartIndices;
    std::vector<MeshInstance> meshInstances;
    std::vector<Mesh *> meshes;
    const BVH *topLevelBvh;
};