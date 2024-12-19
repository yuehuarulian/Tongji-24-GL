// PointCloud.hpp
#ifndef POINT_CLOUD_HPP
#define POINT_CLOUD_HPP

#include <random>
#include <vector>
#include <string>
#include <algorithm>
#include <openvdb/openvdb.h>
#include <openvdb/io/File.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "renderable_model.hpp"
// #include <Eigen/Core>

const float MIN_DENSITY = 0.f;
const float RANDOM_OFFSET = 0.f;

// // Orient2D: 用于计算三点的方向
// struct Orient2D
// {
//     template <typename Scalar>
//     int operator()(const Scalar p1[2], const Scalar p2[2], const Scalar p3[2]) const
//     {
//         // 计算 (p2 - p1) × (p3 - p1) 的 Z 分量，即三点 p1, p2, p3 的方向
//         double det = (p2[0] - p1[0]) * (p3[1] - p1[1]) - (p2[1] - p1[1]) * (p3[0] - p1[0]);
//         // 若 det > 0，则 p1, p2, p3 逆时针；若 det < 0，则顺时针；若 det == 0，则共线
//         if (det > 1e-6)
//             return 1; // 逆时针
//         else if (det < 1e-6)
//             return -1; // 顺时针
//         else
//             return 0; // 共线
//     }
// };
// // InCircle: 用于判断点是否在外接圆内
// struct InCircle
// {
//     template <typename Scalar>
//     int operator()(const Scalar p1[2], const Scalar p2[2], const Scalar p3[2], const Scalar p4[2]) const
//     {
//         // 使用行列式方法判断 p4 是否在三角形 p1, p2, p3 的外接圆内
//         Eigen::Matrix<Scalar, 4, 4> M;
//         M << p1[0], p1[1], p1[0] * p1[0] + p1[1] * p1[1], 1,
//             p2[0], p2[1], p2[0] * p2[0] + p2[1] * p2[1], 1,
//             p3[0], p3[1], p3[0] * p3[0] + p3[1] * p3[1], 1,
//             p4[0], p4[1], p4[0] * p4[0] + p4[1] * p4[1], 1;
//         // 计算行列式
//         double det = M.determinant();
//         // 如果 det > 0，则点 p4 在外接圆内，否则在外接圆外
//         if (det > 1e-6)
//             return 1; // 点在外接圆内
//         else if (det < 1e-6)
//             return -1; // 点在外接圆外
//         else
//             return 0; // 点在外接圆上
//     }
// };

// 计算法线的辅助函数
glm::vec3 compute_normal(const glm::vec3 &p0, const glm::vec3 &p1, const glm::vec3 &p2);

struct Particle
{
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 acceleration;
};

class PointCloud : RenderableModel
{
public:
    PointCloud(const std::string &vdb_file, std::vector<Mesh *> &meshes, std::vector<MeshInstance *> &meshInstances, std::vector<Texture *> &textures, std::vector<Material> &materials, const glm::mat4 &model_matrix);

    ~PointCloud();

    void update() override;

private:
    std::vector<glm::vec3> points;
    std::vector<Particle> particles;

    Material material; // TODO
    Model *model;
    // Orient2D orient2D;
    // InCircle inCircle;

    void set_model_matrix() override;

    void load_vdb_data(const std::string &filename);

    void setup_particles();

    // bool triangulatePointCloudUsingLibIGL();

    void voxelize_point_cloud();

    bool add_model(const std::string &model_path) override;

    glm::vec3 get_velocity_from_field(const glm::vec3 &position);

    void sort_points_by_depth(const glm::vec3 &viewPos);
};

#endif // POINT_CLOUD_HPP