#ifndef MESH_H
#define MESH_H

#include "glad/glad.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
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
    glm::vec3 Position;  // 顶点位置
    glm::vec3 Normal;    // 法线
    glm::vec2 TexCoords; // 纹理坐标
    glm::vec3 Tangent;   // 切线
    glm::vec3 Bitangent; // 副切线

    int m_BoneIDs[MAX_BONE_INFLUENCE];   // 影响该顶点的骨骼索引
    float m_Weights[MAX_BONE_INFLUENCE]; // 每个骨骼的权重
};

struct Texture
{
    unsigned int id;
    string type; // 例如: "texture_diffuse", "texture_specular", "texture_normal", "texture_metallic"
    string path;
};

class Mesh
{
public:
    // 网格数据
    vector<Vertex> vertices;      // 顶点位置、法线方向、纹理坐标
    vector<unsigned int> indices; // 假设所有的面都为三角形 三个索引一个面
    vector<Texture> textures;
    unsigned int VAO;

    BVH *bvh;

    // constructor
    Mesh() = default;
    Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<Texture> textures);

    void BuildBVH();

    unsigned int createDefaultTexture();

    void Draw(Shader &shader);

private:
    // render data
    unsigned int VBO, EBO;

    // initializes all the buffer objects/arrays
    void setupMesh();
};

#endif
