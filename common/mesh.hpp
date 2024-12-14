#ifndef MESH_H
#define MESH_H

#include "glad/glad.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "stb_image.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "shader.hpp"
#include "BVH.hpp"

#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <iostream>
#include <sstream>

#define MAX_BONE_INFLUENCE 4

struct Vertex
{
    // ---------- 顶点数据 ---------- //
    //
    //
    glm::vec3 Position;  // 顶点位置
    glm::vec3 Normal;    // 法线
    glm::vec2 TexCoords; // 纹理坐标
    glm::vec3 Tangent;
    glm::vec3 Bitangent;

    int m_BoneIDs[MAX_BONE_INFLUENCE];   // 影响该顶点的骨骼索引
    float m_Weights[MAX_BONE_INFLUENCE]; // 每个骨骼的权重
};

class Texture
{
    // ---------- 纹理图片数据 ---------- //
    // 图片宽度: width
    // 图片高度：height
    // 图片通道：components
    // 纹理数据：texData -- 根据 width height components 调整大小
    // 纹理名称：texName -- 唯一标识符
public:
    Texture() : width(0), height(0), components(0) {};
    Texture(std::string texName, unsigned char *data, int w, int h, int c) : texName(texName), width(w), height(h), components(c)
    {
        texData.resize(width * height * components);
        std::copy(data, data + width * height * components, texData.begin());
    }
    ~Texture() {}

    bool LoadTexture(const std::string &filename)
    {
        texName = filename;
        components = 4;
        unsigned char *data = stbi_load(filename.c_str(), &width, &height, NULL, components);
        if (data == nullptr)
            return false;
        texData.resize(width * height * components);
        std::copy(data, data + width * height * components, texData.begin());
        stbi_image_free(data);
        if (texData.empty())
        {
            std::cout << "Failed to load texture: " << filename << std::endl;
            return false;
        }
        std::cout << "Texture loaded: " << filename << std::endl;
        return true;
    }

    int width;                          // 纹理的宽度
    int height;                         // 纹理的高度
    int components;                     // 纹理的通道数
    std::vector<unsigned char> texData; // 纹理数据
    std::string texName;                // 纹理名称
};

class Material
{
    // ---------- 材质数据 ---------- //
    // 按照每七个float进行分类 -- 便于将数据传递至Shader中
    // 对于无法补齐的部分可以使用 float padding_i 进行补齐(其中i是一个变量)
public:
    Material()
        : baseColor(glm::vec3(1.0f)), // 默认白色
          specularColor(glm::vec3(0.5f)), // 默认中等强度的镜面反射
          emissiveColor(glm::vec3(0.0f)), // 默认无自发光
          padding_kd(0.0f),
          padding_ks(0.0f),
          padding_ke(0.0f),
          refractionIndex(1.0f),
          transparency(1.0f),
          illuminationModel(1),
          roughness(0.5f), // 默认中等粗糙度
          metalness(0.0f),
          scattering(0.0f),
          coating(0.0f),
          coatRoughness(0.0f),
          diffuseTexId(-1.0f),
          specularTexId(-1.0f),
          normalTexId(-1.0f),
          heightTexId(-1.0f),
          metalnessTexId(-1.0f),
          diffuse_roughnessTexId(-1.0f),
          ambient_occlusionTexId(-1.0f),
          padding_id(0.0f),
          absorption(0.0f),
          density(0.0f),
          anisotropy(0.0f),
          isVolume(0.0f) {}

    void updateTexId(const int offset)
    {
        diffuseTexId += offset;
        specularTexId += offset;
        normalTexId += offset;
        heightTexId += offset;

        metalnessTexId += offset;
        diffuse_roughnessTexId += offset;
        ambient_occlusionTexId += offset;
    }

    // 颜色参数
    // param1
    glm::vec3 baseColor; // 漫反射颜色 (Kd)
    float padding_kd;

    // param2
    glm::vec3 specularColor; // 镜面反射颜色 (Ks)
    float padding_ks;

    // param3
    glm::vec3 emissiveColor; // 自发光颜色 (Ke)
    float padding_ke;

    // 材质参数
    // param4
    float refractionIndex; // 折射率 (Ni)
    float transparency;    // 透明度 (d)
    int illuminationModel; // 光照模型 (illum)
    float roughness;       // 粗糙度 (Pr)

    // param5
    float metalness;     // 金属度 (Pm)
    float scattering;    // 散射系数 (Ps)
    float coating;       // 涂层 (Pc)
    float coatRoughness; // 涂层粗糙度 (Pcr)

    // 纹理贴图的ID
    // param6
    float diffuseTexId;
    float specularTexId;
    float normalTexId;
    float heightTexId;

    // param7
    float metalnessTexId;
    float diffuse_roughnessTexId;
    float ambient_occlusionTexId;
    float padding_id;

    // param8
    float absorption; // 吸收系数 (控制光线穿透的深度)
    float density;    // 密度 (控制光线在材质中的传播)
    float anisotropy; // 各向异性 (控制材质的各向异性)
    float isVolume;   // 是否是体积材质

    void printInfo() const
    {
        std::ostringstream info;
        info << "Material Information:\n";

        // Colors
        info << "Base Color: (" << baseColor.x << ", " << baseColor.y << ", " << baseColor.z << ")\n";
        info << "Specular Color: (" << specularColor.x << ", " << specularColor.y << ", " << specularColor.z << ")\n";
        info << "Emissive Color: (" << emissiveColor.x << ", " << emissiveColor.y << ", " << emissiveColor.z << ")\n";

        // Material properties
        info << "Refraction Index: " << refractionIndex << "\n";
        info << "Transparency: " << transparency << "\n";
        info << "Illumination Model: " << illuminationModel << "\n";
        info << "Roughness: " << roughness << "\n";
        info << "Metalness: " << metalness << "\n";
        info << "Scattering: " << scattering << "\n";
        info << "Coating: " << coating << "\n";
        info << "Coat Roughness: " << coatRoughness << "\n";

        // Texture IDs
        info << "Diffuse Texture ID: " << diffuseTexId << "\n";
        info << "Specular Texture ID: " << specularTexId << "\n";
        info << "Normal Texture ID: " << normalTexId << "\n";
        info << "Height Texture ID: " << heightTexId << "\n";
        info << "Metalness Texture ID: " << metalnessTexId << "\n";
        info << "Diffuse Roughness Texture ID: " << diffuse_roughnessTexId << "\n";
        info << "Ambient Occlusion Texture ID: " << ambient_occlusionTexId << "\n";

        // Additional properties
        info << "Absorption: " << absorption << "\n";
        info << "Density: " << density << "\n";
        info << "Anisotropy: " << anisotropy << "\n";
        info << "Is Volume Material: " << (isVolume != 0.0 ? "Yes" : "No") << "\n";

        // Output to console
        std::cout << info.str();
    }
};

class Mesh
{
public:
    Mesh() = default;

    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, const Material &material);

    void BuildBVH();

    void ProcessVertices(std::vector<glm::vec4> &verticesUVX, std::vector<glm::vec4> &normalsUVY);

    void updateMesh();

    bool needsUpdate(int i);

    std::vector<Vertex> vertices;      // 顶点位置、法线方向、纹理坐标
    std::vector<unsigned int> indices; // 假设所有的面都为三角形 三个索引一个面 indices.size()/3表示三角形的数量
    Material material;
    BVH *bvh;

private:
    bool dirty{false}; // 标志变量，指示是否需要更新
};

class MeshInstance
{
public:
    MeshInstance(int mesh_id, int material_id, glm::mat4 xform)
        : meshID(mesh_id),
          materialID(material_id),
          transform(xform) {}
    ~MeshInstance() {}

    glm::mat4 transform; // 从局部坐标系转换到世界坐标系的转换矩阵

    int meshID;
    int materialID;
};
#endif
