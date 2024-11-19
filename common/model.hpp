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

    bool LoadFromFile(const std::string &filePath) { return loadModel(filePath); }
    std::vector<Mesh *> getMeshes() const { return meshes; }
    std::vector<Texture *> getTextures() const { return textures_loaded; }

private:
    // 模型数据
    vector<Texture *> textures_loaded; // 存储所有的纹理数据
    vector<Mesh *> meshes;             // 存储模型中的所有网格

    string directory;     // 模型文件目录
    bool gammaCorrection; // gamma修正

    bool loadModel(string const &path);                    // 加载模型文件
    void processNode(aiNode *node, const aiScene *scene);  // 递归处理 Scene 节点
    Mesh *processMesh(aiMesh *mesh, const aiScene *scene); // 处理 Mesh
    int loadMaterialTextures(aiMaterial *mat, aiTextureType type);
};

#endif