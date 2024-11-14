#ifndef MODEL_H
#define MODEL_H

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "mesh.hpp"
#include "shader.hpp"
#include "BVH.hpp"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
using namespace std;

class Model
{
public:
    Model() = default;
    Model(string const &path, bool gamma = false);

    // draws the model, and thus all its meshes
    void Draw(Shader &shader);

    // 模型数据
    vector<Texture> textures_loaded; // 用于优化，避免多次加载相同纹理
    vector<Mesh *> meshes;           // 存储模型中的所有网格
    string directory;                // 模型文件目录
    bool gammaCorrection;

    bool LoadFromFile(const std::string &filePath) { return loadModel(filePath); }
    std::vector<Mesh *> getMeshes() const { return meshes; }

private:
    bool loadModel(string const &path);                    // 加载模型文件
    void processNode(aiNode *node, const aiScene *scene);  // 递归处理 Scene 节点
    Mesh *processMesh(aiMesh *mesh, const aiScene *scene); // 处理 Mesh
    vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, string typeName);
};

#endif