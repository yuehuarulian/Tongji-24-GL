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
    vector<Vertex> vertices;
    vector<unsigned int> indices;
    vector<Texture> textures;
    unsigned int VAO;

    // BVH 树
    BVHNode *rootNode;

    // constructor
    Mesh() = default;
    Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<Texture> textures);

    bool LoadFromFile(const std::string &filePath);

    unsigned int createDefaultTexture();

    void Draw(Shader &shader);

private:
    // render data
    unsigned int VBO, EBO;

    void processNode(aiNode *node, const aiScene *scene);
    Mesh processMesh(aiMesh *mesh, const aiScene *scene);

    // initializes all the buffer objects/arrays
    void setupMesh();
};
#endif
