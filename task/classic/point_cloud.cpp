#include "point_cloud.hpp"
// #include <igl/delaunay_triangulation.h>
#include "openvdb/tools/VolumeToMesh.h"

PointCloud::PointCloud(const std::string &vdb_file, std::vector<Mesh *> &meshes, std::vector<MeshInstance *> &meshInstances, std::vector<Texture *> &textures, std::vector<Material> &materials, const glm::mat4 &model_matrix)
    : RenderableModel(meshes, meshInstances, textures, materials)
{
    model = new Model();
    load_vdb_data(vdb_file);
    setup_particles(); // 初始化粒子属性

    this->model_matrix = model_matrix;

    voxelize_point_cloud();
    add_model(vdb_file);
}

PointCloud::~PointCloud()
{
    delete model;
}

void PointCloud::update()
{
    float deltaTime = 1 / 60.0f;
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

            // points.emplace_back(coord.x(), coord.y(), coord.z());
            points.emplace_back(coord.x() + RANDOM_OFFSET * (rand() % 100) / 100.0f,
                                coord.y() + RANDOM_OFFSET * (rand() % 100) / 100.0f,
                                coord.z() + RANDOM_OFFSET * (rand() % 100) / 100.0f);
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

// // 使用 LibIGL 进行 Delaunay 三角化
// bool PointCloud::triangulatePointCloudUsingLibIGL()
// {
//     std::cout << "point cloud triangulatePointCloudUsingLibIGL" << std::endl;
//     // 将 std::vector<glm::vec3> 转换为 Eigen::MatrixXd
//     // 去重共线点
//     // double epsilon = 1e-6; // 容差值，用于判断是否为共线
//     // for (auto it = points.begin(); it != points.end(); ++it)
//     // {
//     //     for (auto jt = it + 1; jt != points.end();)
//     //     {
//     //         // 判断两个点是否足够接近
//     //         if ((it->x - jt->x) * (it->x - jt->x) + (it->y - jt->y) * (it->y - jt->y) < epsilon * epsilon)
//     //             jt = points.erase(jt); // 删除元素并返回新的迭代器
//     //         else
//     //             ++jt; // 只有在不删除的情况下才前进迭代器
//     //     }
//     // }
//     Eigen::MatrixXd V(points.size(), 2); // 这里只使用二维坐标进行三角化
//     for (size_t i = 0; i < points.size(); ++i)
//     {
//         V(i, 0) = points[i].x;
//         V(i, 1) = points[i].y;
//     }
//     // 存储生成的三角形面片
//     Eigen::MatrixXi F;
//     // 使用 LibIGL 的 delaunay_triangulation 进行三角化
//     printf("V.rows():%d\n", V.rows());
//     igl::delaunay_triangulation(V, orient2D, inCircle, F);
//     // 将生成的顶点和面传递到 std::vector
//     std::vector<Vertex> vertices;
//     for (int i = 0; i < V.rows(); ++i)
//     {
//         Vertex vertex;
//         vertex.Position = glm::vec3(V(i, 0), V(i, 1), 0.0f); // 由于是二维点云，z坐标设为0
//         vertex.Normal = glm::vec3(0.0f, 0.0f, 1.0f);         // 初始法线
//         // 可以进一步根据需求计算纹理坐标、切线等
//         vertex.TexCoords = glm::vec2(0.0f, 0.0f);       // 默认纹理坐标
//         vertex.Tangent = glm::vec3(1.0f, 0.0f, 0.0f);   // 默认切线
//         vertex.Bitangent = glm::vec3(0.0f, 1.0f, 0.0f); // 默认双切线
//         vertices.push_back(vertex);
//     }
//     std::vector<unsigned int> indices;
//     std::cout << "F.rows:" << F.rows() << std::endl;
//     for (int i = 0; i < F.rows(); ++i)
//     {
//         std::cout << "F(i):" << F(i, 0) << " " << F(i, 1) << " " << F(i, 2) << std::endl;
//     }
//     for (int i = 0; i < F.rows(); ++i)
//     {
//         // 添加每个三角形的三个顶点
//         indices.push_back(F(i, 0));
//         indices.push_back(F(i, 1));
//         indices.push_back(F(i, 2));
//         // 为每个三角形计算法线
//         glm::vec3 p1 = vertices[F(i, 0)].Position;
//         glm::vec3 p2 = vertices[F(i, 1)].Position;
//         glm::vec3 p3 = vertices[F(i, 2)].Position;
//         glm::vec3 normal = compute_normal(p1, p2, p3);
//         // 为这三个顶点更新法线
//         vertices[F(i, 0)].Normal = normal;
//         vertices[F(i, 1)].Normal = normal;
//         vertices[F(i, 2)].Normal = normal;
//     }
//     model->meshes.push_back(new Mesh(vertices, indices, material));
//     return true;
// }

void PointCloud::voxelize_point_cloud()
{
    // 创建一个空的 FloatGrid
    openvdb::FloatGrid::Ptr grid = openvdb::FloatGrid::create(0.0f);
    grid->setTransform(openvdb::math::Transform::createLinearTransform(1.0)); // 设置体素大小
    grid->setGridClass(openvdb::GRID_FOG_VOLUME);                             // 使用 FOG_VOLUME 类型来表示体素网格

    // 将点云数据转换为体素
    for (const auto &point : points)
    {
        openvdb::Coord xyz(static_cast<int>(point.x), static_cast<int>(point.y), static_cast<int>(point.z));
        grid->tree().setValueOn(xyz, 1.0f); // 设置体素值
    }

    // 使用 VolumeToMesh 工具将体素数据转换为三角形网格
    openvdb::tools::VolumeToMesh mesher(0.5, 0.0); // 使用 isoValue 和 adaptivity
    mesher(*grid);

    // 法线计算部分
    std::unordered_map<unsigned int, glm::vec3> vertex_normals;

    // 将生成的顶点和三角形数据转换为 Mesh 对象
    std::vector<Vertex> mesh_vertices;
    for (size_t i = 0; i < mesher.pointListSize(); ++i)
    {
        const openvdb::Vec3s &v = mesher.pointList()[i];
        Vertex vertex;
        vertex.Position = glm::vec3(v.x(), v.y(), v.z());

        // 计算法线
        vertex.Normal = glm::vec3(0.0f, 0.0f, 1.0f);
        vertex.TexCoords = glm::vec2(0.0f, 0.0f);
        vertex.Tangent = glm::vec3(1.0f, 0.0f, 0.0f);
        vertex.Bitangent = glm::vec3(0.0f, 1.0f, 0.0f);

        mesh_vertices.push_back(vertex);
    }

    // 获取三角形数据
    std::vector<unsigned int> mesh_indices;
    for (size_t i = 0; i < mesher.polygonPoolListSize(); ++i)
    {
        const openvdb::tools::PolygonPool &polygonPool = mesher.polygonPoolList()[i];

        // 处理四边形
        for (size_t j = 0; j < polygonPool.numQuads(); ++j)
        {
            openvdb::Vec4I quad = polygonPool.quad(j);
            mesh_indices.push_back(static_cast<unsigned int>(quad[0]));
            mesh_indices.push_back(static_cast<unsigned int>(quad[1]));
            mesh_indices.push_back(static_cast<unsigned int>(quad[2]));

            // 将四边形拆分为两个三角形
            mesh_indices.push_back(static_cast<unsigned int>(quad[0]));
            mesh_indices.push_back(static_cast<unsigned int>(quad[2]));
            mesh_indices.push_back(static_cast<unsigned int>(quad[3]));

            // 为每个三角形计算法线
            glm::vec3 p0 = mesh_vertices[quad[0]].Position;
            glm::vec3 p1 = mesh_vertices[quad[1]].Position;
            glm::vec3 p2 = mesh_vertices[quad[2]].Position;
            glm::vec3 p3 = mesh_vertices[quad[3]].Position;

            glm::vec3 normal1 = compute_normal(p0, p1, p2);
            glm::vec3 normal2 = compute_normal(p0, p2, p3);

            vertex_normals[quad[0]] += normal1;
            vertex_normals[quad[1]] += normal1;
            vertex_normals[quad[2]] += normal1;
            vertex_normals[quad[3]] += normal2;
        }

        // 处理三角形
        for (size_t j = 0; j < polygonPool.numTriangles(); ++j)
        {
            openvdb::Vec3I triangle = polygonPool.triangle(j);
            mesh_indices.push_back(static_cast<unsigned int>(triangle[0]));
            mesh_indices.push_back(static_cast<unsigned int>(triangle[1]));
            mesh_indices.push_back(static_cast<unsigned int>(triangle[2]));

            // 为每个三角形计算法线
            glm::vec3 p0 = mesh_vertices[triangle[0]].Position;
            glm::vec3 p1 = mesh_vertices[triangle[1]].Position;
            glm::vec3 p2 = mesh_vertices[triangle[2]].Position;

            glm::vec3 normal = compute_normal(p0, p1, p2);

            vertex_normals[triangle[0]] += normal;
            vertex_normals[triangle[1]] += normal;
            vertex_normals[triangle[2]] += normal;
        }
    }

    // 平滑法线：归一化法线
    for (auto &vertex : mesh_vertices)
    {
        vertex.Normal = glm::normalize(vertex_normals[&vertex - &mesh_vertices[0]]);
    }

    // 创建新的 Mesh 对象并添加到模型中
    model->meshes.push_back(new Mesh(mesh_vertices, mesh_indices, material));
    return;
}

bool PointCloud::add_model(const std::string &model_path)
{
    std::cout << "point cloud add_model start" << std::endl;
    // 1. 将model中的纹理数据导入scene中
    int textureStartId = this->textures.size();
    // for (auto texture : model->getTextures())
    //     this->textures.push_back(texture);

    // 2. 将model中的网格数据导入scene中
    for (auto mesh : model->getMeshes())
    {
        int mesh_id = meshes.size();
        int materialStartId = this->materials.size();

        // 3. 将mesh中的材质信息导入scene中
        //    同时更新纹理索引
        mesh->material.updateTexId(textureStartId);
        this->materials.push_back(mesh->material);
        // 4. 根据网格id和材质id创建一个meshInstance
        MeshInstance *instance = new MeshInstance(mesh_id, materialStartId, model_matrix);

        this->meshes.push_back(mesh);
        this->meshInstances.push_back(instance);
        std::cout << "point cloud index.size():" << mesh->indices.size() << std::endl;
    }
    std::cout << "point cloud add_model end" << std::endl;
    return true;
}

void PointCloud::set_model_matrix()
{
    model_matrix = glm::mat4(1.0f);
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

// 计算法线的辅助函数
glm::vec3 compute_normal(const glm::vec3 &p0, const glm::vec3 &p1, const glm::vec3 &p2)
{
    glm::vec3 v1 = p1 - p0;
    glm::vec3 v2 = p2 - p0;
    return glm::normalize(glm::cross(v1, v2)); // 叉积得到法向量
}
