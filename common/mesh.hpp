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

using namespace std;

struct Vertex
{
    // ---------- 顶点数据 ---------- //
    //
    //
    glm::vec3 Position;  // 顶点位置
    glm::vec3 Normal;    // 法线
    glm::vec2 TexCoords; // 纹理坐标
    glm::vec3 Tangent;   // 切线
    glm::vec3 Bitangent; // 副切线

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
    Texture(std::string texName, unsigned char *data, int w, int h, int c) : texName(texName),
                                                                             width(w),
                                                                             height(h),
                                                                             components(c)

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
    // 按照每四个float进行分类 -- 便于将数据传递至Shader中
    // 对于无法补齐的部分可以使用 float padding_i 进行补齐(其中i是一个变量)
public:
    Material()
        : baseColor(glm::vec3(0.8824, 0.0627, 0.0627)),
          diffuseTexId(-1.0f),
          specularTexId(-1.0f),
          normalTexId(-1.0f),
          heightTexId(-1.0f),
          metalnessTexId(-1.0f),
          diffuse_roughnessTexId(-1.0f),
          ambient_occlusionTexId(-1.0f) {}

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

    glm::vec3 baseColor; // 基础颜色
    float padding_0;

    float diffuseTexId;
    float specularTexId;
    float normalTexId;
    float heightTexId;

    float metalnessTexId;
    float diffuse_roughnessTexId;
    float ambient_occlusionTexId;
    float padding_1;
};

class Mesh
{
public:
    // 网格数据
    vector<Vertex> vertices;      // 顶点位置、法线方向、纹理坐标
    vector<unsigned int> indices; // 假设所有的面都为三角形 三个索引一个面 indices.size()/3表示三角形的数量
    Material material;
    BVH *bvh;

    // 构造函数
    Mesh() = default;
    Mesh(vector<Vertex> vertices, vector<unsigned int> indices, const Material &material);

    void BuildBVH();
    void ProcessVertices(std::vector<glm::vec4> &verticesUVX, std::vector<glm::vec4> &normalsUVY);

    unsigned int createDefaultTexture();

private:
    // initializes all the buffer objects/arrays
    void setupMesh();
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
