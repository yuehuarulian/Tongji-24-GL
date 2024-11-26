#include <BVH.hpp>
#include <iostream>
#include <algorithm>

static int constexpr kMaxPrimitivesPerLeaf = 1;

void BVH::InitNodeAllocator(size_t maxnum)
{
    m_nodecnt = 0;          // 初始化节点分配器
    m_nodes.resize(maxnum); // 预分配最大数量的节点
}

BVH::Node *BVH::AllocateNode()
{
    // 分配一个新的节点
    return &m_nodes[m_nodecnt++]; // 返回当前节点并增加节点计数
}

void BVH::Build(AABB const *bounds, int numbounds)
{
    // 构建BVH树，遍历每个边界框并更新总的边界框（m_bounds）
    for (int i = 0; i < numbounds; ++i)
        m_bounds.grow(bounds[i]); // 计算边界框并扩展总边界框

    // 调用BuildImpl进行具体的BVH构建
    BuildImpl(bounds, numbounds);
}

void BVH::BuildImpl(AABB const *bounds, int numbounds)
{
    // 初始化节点分配器，预分配最多 2 * numbounds - 1 个节点，确保足够的空间来构建树
    InitNodeAllocator(2 * numbounds - 1);

    // 缓存一些数据
    std::vector<glm::vec3> centroids(numbounds);      // 存储所有bbox的质心
    m_indices.resize(numbounds);                      // 为每个bbox分配一个索引
    std::iota(m_indices.begin(), m_indices.end(), 0); // 用连续的整数填充索引

    AABB centroid_bounds;
    for (size_t i = 0; i < static_cast<size_t>(numbounds); ++i)
    {
        // 计算每个物体的质心
        glm::vec3 c = bounds[i].getCenter();
        centroid_bounds.grow(c); // 将每个物体的质心扩展到质心边界
        centroids[i] = c;        // 存储质心
    }

    // 初始化分割请求（开始分割的起始索引、物体数量、树的根节点等信息）
    SplitRequest init = {0, numbounds, nullptr, m_bounds, centroid_bounds, 0, 1};

    BuildNode(init, bounds, &centroids[0], &m_indices[0]);

    m_root = &m_nodes[0];
}

void BVH::BuildNode(SplitRequest const &req, AABB const *bounds, glm::vec3 const *centroids, int *primindices)
{
    // 更新树的高度
    m_height = std::max(m_height, req.level);

    Node *node = AllocateNode(); // 为当前节点分配空间
    node->bounds = req.bounds;   // 设置节点的边界框
    node->index = req.index;     // 设置节点的索引

    // 如果原始物体数量小于 2，则创建叶节点
    if (req.numprims < 2)
    {
        node->type = kLeaf;                                         // 设置节点类型为叶节点
        node->startidx = static_cast<int>(m_packed_indices.size()); // 记录叶节点的原始索引起始位置
        node->numprims = req.numprims;                              // 记录叶节点包含的原始物体数量

        // 将原始物体的索引添加到 packed_indices 中
        for (auto i = 0; i < req.numprims; ++i)
        {
            m_packed_indices.push_back(primindices[req.startidx + i]);
        }
    }
    else
    {
        // 设置节点类型为内部节点
        node->type = kInternal;

        // 选择最大方向进行分割
        int axis = req.centroid_bounds.maxdim();              // 选择最大方向的轴
        float border = req.centroid_bounds.getCenter()[axis]; // 计算该轴的中心位置作为分割边界

        if (m_usesah)
        {
            // 使用 SAH（Surface Area Heuristic）算法进行分割
            SahSplit ss = FindSahSplit(req, bounds, centroids, primindices);

            // 如果找到了有效的分割边界
            if (!std::isnan(ss.split))
            {
                axis = ss.dim;     // 设置分割的轴
                border = ss.split; // 设置分割的边界

                // 如果分割后的物体数小于阈值，直接创建叶节点
                if (req.numprims < ss.sah && req.numprims < kMaxPrimitivesPerLeaf)
                {
                    node->type = kLeaf;
                    node->startidx = static_cast<int>(m_packed_indices.size());
                    node->numprims = req.numprims;

                    for (auto i = 0; i < req.numprims; ++i)
                    {
                        m_packed_indices.push_back(primindices[req.startidx + i]);
                    }

                    if (req.ptr)
                        *req.ptr = node; // 设置父节点指针
                    return;              // 返回
                }
            }
        }

        // 初始化左右子树的边界框
        AABB leftbounds, rightbounds, leftcentroid_bounds, rightcentroid_bounds;
        int splitidx = req.startidx; // 初始化分割索引

        bool near2far = (req.numprims + req.startidx) & 0x1; // 用于决定分割的方向

        // 如果最大轴方向的边界框宽度大于 0，则进行分割
        if (req.centroid_bounds.getExtent()[axis] > 0.0f)
        {
            auto first = req.startidx;
            auto last = req.startidx + req.numprims;

            // 根据分割方向进行物体的分割
            if (near2far)
            {
                while (true)
                {
                    while ((first != last) && centroids[primindices[first]][axis] < border)
                    {
                        leftbounds.grow(bounds[primindices[first]]);
                        leftcentroid_bounds.grow(centroids[primindices[first]]);
                        ++first;
                    }

                    if (first == last--)
                        break;

                    rightbounds.grow(bounds[primindices[first]]);
                    rightcentroid_bounds.grow(centroids[primindices[first]]);

                    while ((first != last) && centroids[primindices[last]][axis] >= border)
                    {
                        rightbounds.grow(bounds[primindices[last]]);
                        rightcentroid_bounds.grow(centroids[primindices[last]]);
                        --last;
                    }

                    if (first == last)
                        break;

                    leftbounds.grow(bounds[primindices[last]]);
                    leftcentroid_bounds.grow(centroids[primindices[last]]);

                    std::swap(primindices[first++], primindices[last]);
                }
            }
            else
            {
                while (true)
                {
                    while ((first != last) && centroids[primindices[first]][axis] >= border)
                    {
                        leftbounds.grow(bounds[primindices[first]]);
                        leftcentroid_bounds.grow(centroids[primindices[first]]);
                        ++first;
                    }

                    if (first == last--)
                        break;

                    rightbounds.grow(bounds[primindices[first]]);
                    rightcentroid_bounds.grow(centroids[primindices[first]]);

                    while ((first != last) && centroids[primindices[last]][axis] < border)
                    {
                        rightbounds.grow(bounds[primindices[last]]);
                        rightcentroid_bounds.grow(centroids[primindices[last]]);
                        --last;
                    }

                    if (first == last)
                        break;

                    leftbounds.grow(bounds[primindices[last]]);
                    leftcentroid_bounds.grow(centroids[primindices[last]]);

                    std::swap(primindices[first++], primindices[last]);
                }
            }

            splitidx = first; // 更新分割索引
        }

        // 如果分割索引没有变化，则使用简单的中位数分割
        if (splitidx == req.startidx || splitidx == req.startidx + req.numprims)
        {
            splitidx = req.startidx + (req.numprims >> 1);

            for (int i = req.startidx; i < splitidx; ++i)
            {
                leftbounds.grow(bounds[primindices[i]]);
                leftcentroid_bounds.grow(centroids[primindices[i]]);
            }

            for (int i = splitidx; i < req.startidx + req.numprims; ++i)
            {
                rightbounds.grow(bounds[primindices[i]]);
                rightcentroid_bounds.grow(centroids[primindices[i]]);
            }
        }

        // 构建左右子树的请求
        SplitRequest leftrequest = {req.startidx, splitidx - req.startidx, &node->leftChild, leftbounds, leftcentroid_bounds, req.level + 1, (req.index << 1)};
        SplitRequest rightrequest = {splitidx, req.numprims - (splitidx - req.startidx), &node->rightChild, rightbounds, rightcentroid_bounds, req.level + 1, (req.index << 1) + 1};

        // 递归构建左右子树
        BuildNode(leftrequest, bounds, centroids, primindices);
        BuildNode(rightrequest, bounds, centroids, primindices);
    }

    // 设置父节点指针
    if (req.ptr)
        *req.ptr = node;
}

// 通过使用 Surface Area Heuristic (SAH) 算法来寻找最优的分割方式
// 返回一个 SahSplit 结构体，包含最佳分割轴、分割位置、SAH值等信息
BVH::SahSplit BVH::FindSahSplit(SplitRequest const &req, AABB const *bounds, glm::vec3 const *centroids, int *primindices) const
{
    int splitidx = -1;                             // 初始化分割索引
    float sah = std::numeric_limits<float>::max(); // 设置初始 SAH 为最大值
    SahSplit split;
    split.dim = 0;
    split.split = std::numeric_limits<float>::quiet_NaN(); // 初始分割位置为 NaN，表示未设置

    // 如果物体的质心范围在某个维度上为零，说明无法在该维度上进行有效分割
    glm::vec3 centroid_extents = req.centroid_bounds.getExtent();
    if (glm::dot(centroid_extents, centroid_extents) == 0.f)
    {
        return split;
    }

    // 定义一个 Bin 结构，用于存储每个 bin 的边界框和出现次数
    struct Bin
    {
        AABB bounds; // 当前 bin 的边界框
        int count;   // 当前 bin 中包含的物体数量
    };

    // 为每个维度分别准备三个 bin 数组 -- 分区
    std::vector<Bin> bins[3];
    bins[0].resize(m_num_bins); // 维度 0（x轴）的 bin 数组
    bins[1].resize(m_num_bins); // 维度 1（y轴）的 bin 数组
    bins[2].resize(m_num_bins); // 维度 2（z轴）的 bin 数组

    float invarea = 1.f / req.bounds.getSurfaceArea(); // 计算逆父框面积（用于后续计算 SAH）
    glm::vec3 rootmin = req.centroid_bounds.getMinP(); // 计算最小点位置（用于后续计算分割范围）

    // 遍历所有三个维度（x、y、z），寻找最优的分割轴
    for (int axis = 0; axis < 3; ++axis)
    {
        float rootminc = rootmin[axis];              // 当前维度的最小值
        float centroid_rng = centroid_extents[axis]; // 当前维度的质心范围
        float invcentroid_rng = 1.f / centroid_rng;  // 当前维度质心范围的倒数

        // 如果当前维度的质心范围为零，跳过该维度
        if (centroid_rng == 0.f)
            continue;

        // 初始化当前维度的所有 bin（每个 bin 的计数和边界框）
        for (int i = 0; i < m_num_bins; ++i)
        {
            bins[axis][i].count = 0;
            bins[axis][i].bounds = AABB();
        }

        // 计算物体在当前维度上的直方图
        for (int i = req.startidx; i < req.startidx + req.numprims; ++i)
        {
            int idx = primindices[i]; // 当前物体的索引
            int binidx = (int)std::min<float>(static_cast<float>(m_num_bins) * ((centroids[idx][axis] - rootminc) * invcentroid_rng), static_cast<float>(m_num_bins - 1));

            // 将当前物体放入对应的 bin 中
            ++bins[axis][binidx].count;
            bins[axis][binidx].bounds.grow(bounds[idx]);
        }

        // 创建一个数组存储右侧每个 bin 的边界框
        std::vector<AABB> rightbounds(m_num_bins - 1);

        // 从右到左填充 rightbounds 数组
        AABB rightbox = AABB();
        for (int i = m_num_bins - 1; i > 0; --i)
        {
            rightbox.grow(bins[axis][i].bounds); // 将当前 bin 的边界框添加到 rightbox 中
            rightbounds[i - 1] = rightbox;       // 存储当前 bin 的右侧边界框
        }

        // 初始化左侧边界框和计数器
        AABB leftbox = AABB();
        int leftcount = 0;
        int rightcount = req.numprims;

        // 开始搜索最优的 SAH 分割点
        float sahtmp = 0.f;
        for (int i = 0; i < m_num_bins - 1; ++i)
        {
            // 更新左侧边界框和右侧计数器
            leftbox.grow(bins[axis][i].bounds);
            leftcount += bins[axis][i].count;
            rightcount -= bins[axis][i].count;

            // 计算 SAH 值
            sahtmp = m_traversal_cost + (leftcount * leftbox.getSurfaceArea() + rightcount * rightbounds[i].getSurfaceArea()) * invarea;

            // 如果当前 SAH 值比之前找到的值更小，更新分割信息
            if (sahtmp < sah)
            {
                split.dim = axis;         // 设置最佳分割维度
                splitidx = i;             // 设置最佳分割索引
                split.sah = sah = sahtmp; // 更新最小 SAH 值
            }
        }
    }

    // 如果找到了有效的分割点，计算分割位置
    if (splitidx != -1)
    {
        // 计算分割位置：在当前维度的分割位置为 rootmin[dim] + (splitidx + 1) * (centroid_extents[dim] / m_num_bins)
        split.split = rootmin[split.dim] + (splitidx + 1) * (centroid_extents[split.dim] / m_num_bins);
    }

    // 返回最优分割信息
    return split;
}

void BVH::PrintStatistics(std::ostream &os) const
{
    os << "BVH INFO: \n";
    os << "SAH: " << (m_usesah ? "enabled\n" : "disabled\n");
    os << "SAH bins: " << m_num_bins << "\n";
    os << "Number of triangles: " << m_indices.size() << "\n";
    os << "Number of nodes: " << m_nodecnt << "\n";
    os << "Tree height: " << getHeight() << "\n";
}