#include "point_cloud.hpp"

PointCloud::PointCloud(const std::string &vdb_file, std::shared_ptr<Shader> shader) : shader(std::move(shader))
{
    load_vdb_data(vdb_file);
    setup_particles(); // 初始化粒子属性
    setup_opengl();
}

PointCloud::~PointCloud()
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}

void PointCloud::draw(const glm::mat4 &projection, const glm::mat4 &view,
                      const glm::vec3 &viewPos, const glm::vec3 &fogColor, float fogDensity, bool sort_points, bool updata_particles, float deltatime)
{
    // 在渲染之前进行深度排序
    if (sort_points)
    {
        sort_points_by_depth(viewPos);
    }

    if (updata_particles)
    {
        update(deltatime);
    }

    // 更新到 GPU
    if (sort_points || updata_particles)
    {
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(glm::vec3), points.data(), GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

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

void PointCloud::update(float deltaTime)
{
    for (auto &particle : particles)
    {
        // 模拟流场中的粒子运动
        glm::vec3 velocity = get_velocity_from_field(particle.position);
        particle.velocity += velocity * deltaTime;
        particle.position += particle.velocity * deltaTime;

        // 添加外力（如重力和阻力）
        particle.acceleration = glm::vec3(0.0f, -9.8f, 0.0f) - particle.velocity * 0.1f;
    }

    // 将粒子的位置更新到点云数据
    points.clear();
    for (const auto &particle : particles)
    {
        points.push_back(particle.position);
    }
}

void PointCloud::load_vdb_data(const std::string &filename)
{
    openvdb::initialize();
    openvdb::io::File file(filename);
    file.open();

    openvdb::GridBase::Ptr baseGrid;

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

            points.emplace_back(coord.x(), coord.y(), coord.z());
        }
    }
    file.close();
}

void PointCloud::setup_particles()
{
    // 初始化粒子，使用点云作为初始位置
    for (const auto &point : points)
    {
        Particle particle;
        particle.position = point;
        particle.velocity = glm::vec3(0.0f);
        particle.acceleration = glm::vec3(0.0f);
        particles.push_back(particle);
    }
}

glm::vec3 PointCloud::get_velocity_from_field(const glm::vec3 &position)
{
    // 示例速度场：简单的漩涡效果
    float radius = glm::length(position);
    if (radius < 1e-3)
        return glm::vec3(0.0f); // 避免除零

    glm::vec3 tangent(-position.y, position.x, 0.0f);
    tangent = glm::normalize(tangent);
    return tangent * 0.1f; // 控制速度大小
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
