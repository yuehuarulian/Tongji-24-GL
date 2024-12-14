#include <BVHConverter.hpp>

/// 处理 BLAS 和 TLAS 数据，生成完整的索引数据。
void BVHConverter::Process(const BVH *topLevelBvh, const std::vector<Mesh *> &sceneMeshes, const std::vector<MeshInstance* > &sceneInstances)
{
    this->topLevelBvh = topLevelBvh; // 顶层的BVH树
    meshes = sceneMeshes;            // 所有的网格数据
    meshInstances = sceneInstances;  // 所有的顶层网格数据

    ProcessBLAS(); // 先处理 BLAS 数据
    ProcessTLAS(); // 再处理 TLAS 数据
}

/// 处理 BLAS（底层加速结构）数据
/// 这个函数会遍历所有的网格并处理它们的 BVH 数据，
void BVHConverter::ProcessBLAS()
{
    int nodeCnt = 0;

    // 计算所有网格的节点总数
    for (int i = 0; i < meshes.size(); i++)
        nodeCnt += meshes[i]->bvh->getNodeCnt(); // 获取每个Mesh对应BVH树包含的节点数量

    // 设置顶层 BVH 索引为节点总数
    topLevelIndex = nodeCnt;

    // 为顶层节点预留空间
    nodes.clear();
    nodeCnt += 2 * meshInstances.size();
    nodes.resize(nodeCnt);

    int bvhRootIndex = 0;
    curTriIndex = 0;

    // 处理每个网格的 BLAS 数据
    for (int i = 0; i < meshes.size(); i++)
    {
        Mesh *mesh = meshes[i];
        curNode = bvhRootIndex;

        // 记录当前网格的 BVH 根节点的起始位置
        bvhRootStartIndices.push_back(bvhRootIndex);
        bvhRootIndex += mesh->bvh->getNodeCnt();

        // 递归处理当前网格的 BLAS 节点
        ProcessBLASNodes(mesh->bvh->getRootNode());
        curTriIndex += mesh->bvh->getNumIndices(); // 更新三角形索引
    }
}
/// 处理 TLAS（顶层加速结构）数据
/// 处理顶层加速结构的节点，生成最终的索引数据。
void BVHConverter::ProcessTLAS()
{
    curNode = topLevelIndex;
    ProcessTLASNodes(topLevelBvh->getRootNode());
}

/// 处理底层加速结构 (BLAS) 节点的递归函数
/// 将 BLAS 节点的包围盒和信息转换成适合 GPU 使用的索引数据结构。
int BVHConverter::ProcessBLASNodes(const BVH::Node *node)
{
    // 获取当前节点的包围盒
    AABB bbox = node->bounds;

    // 将当前节点的包围盒最小值和最大值存储到节点数组中
    nodes[curNode].bboxmin = bbox.getMinP();
    nodes[curNode].bboxmax = bbox.getMaxP();

    // 初始化 LRLeaf.z 为 0，表示该节点不是叶子节点
    nodes[curNode].LRLeaf.z = 0;

    // 当前节点的索引
    int index = curNode;

    // 如果是叶子节点
    if (node->type == BVH::NodeType::kLeaf)
    {
        // 对于叶子节点，设置 LRLeaf.x 和 LRLeaf.y，表示三角形索引和三角形数量
        nodes[curNode].LRLeaf.x = curTriIndex + node->startidx; // 计算三角形的索引
        nodes[curNode].LRLeaf.y = node->numprims;               // 存储该节点包含的三角形数量
        nodes[curNode].LRLeaf.z = 1;                            // 标记为叶子节点
    }
    else
    {
        // 如果是内部节点，递归处理左子树和右子树
        curNode++;                                                  // 处理左子树
        nodes[index].LRLeaf.x = ProcessBLASNodes(node->leftChild);  // 递归处理左子节点
        curNode++;                                                  // 处理右子树
        nodes[index].LRLeaf.y = ProcessBLASNodes(node->rightChild); // 递归处理右子节点
    }

    // 返回当前节点的索引
    return index;
}
/// 处理顶层加速结构 (TLAS) 节点的递归函数
/// 将 TLAS 节点的包围盒和信息转换成适合 GPU 使用的索引数据结构。
int BVHConverter::ProcessTLASNodes(const BVH::Node *node)
{
    // 获取当前节点的包围盒
    AABB bbox = node->bounds;

    // 将当前节点的包围盒最小值和最大值存储到节点数组中
    nodes[curNode].bboxmin = bbox.getMinP();
    nodes[curNode].bboxmax = bbox.getMaxP();

    // 初始化 LRLeaf.z 为 0，表示该节点不是叶子节点
    nodes[curNode].LRLeaf.z = 0;

    // 当前节点的索引
    int index = curNode;

    // 如果是叶子节点
    if (node->type == BVH::NodeType::kLeaf)
    {
        // 获取该叶子节点对应的实例索引
        int instanceIndex = topLevelBvh->m_packed_indices[node->startidx];

        // 获取网格索引和材质ID
        int meshIndex = meshInstances[instanceIndex]->meshID;
        int materialID = meshInstances[instanceIndex]->materialID;

        // 将网格的根节点开始索引存入 LRLeaf.x
        nodes[curNode].LRLeaf.x = bvhRootStartIndices[meshIndex];
        // 将材质ID存入 LRLeaf.y
        nodes[curNode].LRLeaf.y = materialID;
        // 将实例的负索引存入 LRLeaf.z
        nodes[curNode].LRLeaf.z = -instanceIndex - 1;
    }
    else
    {
        // 如果是内部节点，递归处理左子树和右子树
        curNode++;                                                  // 处理左子树
        nodes[index].LRLeaf.x = ProcessTLASNodes(node->leftChild);  // 递归处理左子节点
        curNode++;                                                  // 处理右子树
        nodes[index].LRLeaf.y = ProcessTLASNodes(node->rightChild); // 递归处理右子节点
    }

    // 返回当前节点的索引
    return index;
}