#include "load_model.h"

#include <fluid/math/constants.h>
#include <fluid/renderer/camera.h>
#include <fluid/renderer/material.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
//debug
#include <iostream>
#include <stdexcept>

using namespace fluid;
using namespace fluid::renderer;

LoadModel::LoadModel()
    : filepath_(""), scale_(0.0), offset_(vec3d()),
    // 初始化最大值和最小值
    min_coords_(std::numeric_limits<double>::max(),
        std::numeric_limits<double>::max(),
        std::numeric_limits<double>::max()),
    max_coords_(std::numeric_limits<double>::lowest(),
        std::numeric_limits<double>::lowest(),
        std::numeric_limits<double>::lowest()) {};

LoadModel::LoadModel(const std::string& filepath, double scale, const vec3d& offset)
    : filepath_(filepath), scale_(scale), offset_(offset), 
    // 初始化最大值和最小值
    min_coords_(std::numeric_limits<double>::max(),
        std::numeric_limits<double>::max(),
        std::numeric_limits<double>::max()),
    max_coords_(std::numeric_limits<double>::lowest(),
        std::numeric_limits<double>::lowest(),
        std::numeric_limits<double>::lowest()) 
{
    // 加载模型
    load();
    print_mesh_info();
}

void LoadModel::addModel(const std::string& filepath, double scale, const vec3d& offset)
{
    // 设置参数
    filepath_ = filepath;
    scale_ = scale;
    offset_ = offset;
    // 加载模型
    load();
}

bool LoadModel::load() {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(filepath_, aiProcess_Triangulate | aiProcess_GenNormals);

    if (!scene || !scene->HasMeshes()) {
        std::cerr << "Failed to load model file: " << filepath_ << std::endl;
        return false;
    }

    std::cout << "Loading File: " << filepath_ << std::endl;
    std::cout << "Number of meshes: " << scene->mNumMeshes << std::endl;

    for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
        aiMesh* aiMesh = scene->mMeshes[i];

        std::cout << "Processing mesh " << i + 1 << " / " << scene->mNumMeshes
            << " - Vertices: " << aiMesh->mNumVertices
            << ", Faces: " << aiMesh->mNumFaces << std::endl;

        // 加载并应用缩放和偏移到顶点，同时计算最值
        for (unsigned int j = 0; j < aiMesh->mNumVertices; ++j) {
            vec3d vertex(
                aiMesh->mVertices[j].y * scale_ + offset_.x,
                aiMesh->mVertices[j].x * scale_ + offset_.y,
                aiMesh->mVertices[j].z * scale_ + offset_.z
            );
            mesh_.positions.push_back(vertex);

            min_coords_.x = std::min(min_coords_.x, vertex.x);
            min_coords_.y = std::min(min_coords_.y, vertex.y);
            min_coords_.z = std::min(min_coords_.z, vertex.z);
            max_coords_.x = std::max(max_coords_.x, vertex.x);
            max_coords_.y = std::max(max_coords_.y, vertex.y);
            max_coords_.z = std::max(max_coords_.z, vertex.z);

            if (aiMesh->HasNormals()) {
                mesh_.normals.emplace_back(aiMesh->mNormals[j].x, aiMesh->mNormals[j].y, aiMesh->mNormals[j].z);
            }
        }

        // 加载面信息
        for (unsigned int j = 0; j < aiMesh->mNumFaces; ++j) {
            const aiFace& face = aiMesh->mFaces[j];
            for (unsigned int k = 0; k < face.mNumIndices; ++k) {
                mesh_.indices.push_back(face.mIndices[k]);
            }
        }
    }

    // 更新size信息
    size_coords_ = max_coords_ - min_coords_;
    std::cout << "size_coords_: (" << size_coords_.x << " " << size_coords_.y << " " << size_coords_.z << ")" << std::endl;

    std::cout << "Successfully loaded model file: " << filepath_ << std::endl;
    std::cout << std::endl;
    return true;
}

const renderer::scene::mesh_t& LoadModel::get_mesh() const {
    return mesh_;
}

const vec3d& LoadModel::get_offset() const {
    return min_coords_;
}

const vec3d& LoadModel::get_size() const {
    return size_coords_;
}

void LoadModel::print_mesh_info() const {
    std::cout << "Model file: " << filepath_ << std::endl;
    std::cout << "Scale: " << scale_ << ", Offset: (" << offset_.x << ", " << offset_.y << ", " << offset_.z << ")" << std::endl;
    std::cout << "Mesh vertices: " << mesh_.positions.size() << ", faces: " << mesh_.indices.size() / 3 << std::endl;
    std::cout << "Bounding box after scaling and offset:" << std::endl;
    std::cout << "  Min (x, y, z): (" << min_coords_.x << ", " << min_coords_.y << ", " << min_coords_.z << ")" << std::endl;
    std::cout << "  Max (x, y, z): (" << max_coords_.x << ", " << max_coords_.y << ", " << max_coords_.z << ")" << std::endl;
    std::cout << "  Size = (" << size_coords_.x << ", " << size_coords_.y << ", " << size_coords_.z << ")" << std::endl;
    std::cout << std::endl;
}