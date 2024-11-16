// PointCloud.cpp
#include "point_cloud.hpp"

PointCloud::PointCloud(const std::string &vdb_file, std::shared_ptr<Shader> shader) : shader(std::move(shader))
{
    load_vdb_data(vdb_file);
    setup_opengl();
}

PointCloud::~PointCloud()
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}

void PointCloud::draw(const glm::mat4 &projection, const glm::mat4 &view,
                      const glm::vec3 &viewPos, const glm::vec3 &fogColor, float fogDensity, bool sort_points)
{
    // 在渲染之前进行深度排序
    if (sort_points)
        sort_points_by_depth(viewPos);

    // 重新更新点云数据到 GPU
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(glm::vec3), points.data(), GL_STATIC_DRAW);

    shader->use();
    shader->setMat4("model", model);
    shader->setMat4("view", view);
    shader->setMat4("projection", projection);

    // 设置雾相关的
    shader->setVec3("viewPos", viewPos);
    shader->setVec3("fogColor", fogColor);
    shader->setFloat("fogDensity", fogDensity);

    // 绑定 VAO 并绘制点云
    glBindVertexArray(VAO);
    glDrawArrays(GL_POINTS, 0, points.size());
}

void PointCloud::load_vdb_data(const std::string &filename)
{
    openvdb::initialize();
    openvdb::io::File file(filename);
    file.open();

    openvdb::GridBase::Ptr baseGrid;

    // 用于生成随机扰动的随机数引擎
    std::default_random_engine generator;
    std::uniform_real_distribution<float> distribution(RANDOM_OFFSET, RANDOM_OFFSET); // 随机偏移范围

    for (openvdb::io::File::NameIterator nameIter = file.beginName(); nameIter != file.endName(); ++nameIter)
    {
        baseGrid = file.readGrid(nameIter.gridName());
        openvdb::FloatGrid::Ptr floatGrid = openvdb::gridPtrCast<openvdb::FloatGrid>(baseGrid);

        if (!floatGrid)
        {
            continue;
        }

        for (openvdb::FloatGrid::ValueOnCIter iter = floatGrid->cbeginValueOn(); iter.test(); ++iter)
        {
            if (iter.getValue() < MIN_DENSITY) // 过滤掉密度较小的点
                continue;
            openvdb::Coord coord = iter.getCoord();

            // 添加随机扰动，使点的分布更自然
            float offsetX = distribution(generator);
            float offsetY = distribution(generator);
            float offsetZ = distribution(generator);

            points.emplace_back(coord.x() + offsetX, coord.y() + offsetY, coord.z() + offsetZ);
        }
    }
    file.close();
}

void PointCloud::setup_opengl()
{
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(glm::vec3), points.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void *)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void PointCloud::sort_points_by_depth(const glm::vec3 &viewPos)
{
    // 使用 lambda 表达式对点进行排序，依据是点到相机位置的距离,按距离从远到近排序
    std::sort(points.begin(), points.end(), [&viewPos](const glm::vec3 &a, const glm::vec3 &b)
              {
                  float distA = glm::length(a - viewPos);
                  float distB = glm::length(b - viewPos);
                  return distA > distB; });
}
