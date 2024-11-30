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
#include "assimp_glm_helpers.hpp"
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
    // constructor, expects a filepath to a 3D model.
    Model() = default;

    Model(string const &path, bool gamma = false);

    bool LoadFromFile(const std::string &filePath) { return loadModel(filePath); }

    std::vector<Mesh *> getMeshes() const { return meshes; }

    std::vector<Texture *> getTextures() const { return textures_loaded; }

    vector<Mesh *> meshes; // 存储模型中的所有网格

private:
    // 模型数据
    vector<Texture *> textures_loaded; // 存储所有的纹理数据

    string directory;     // 模型文件目录
    bool gammaCorrection; // gamma修正

    // loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
    bool loadModel(string const &path);

    // processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
    void processNode(aiNode *node, const aiScene *scene);

    Mesh *processMesh(aiMesh *mesh, const aiScene *scene);

    // checks all material textures of a given type and loads the textures if they're not loaded yet.
    // the required info is returned as a Texture struct.
    int loadMaterialTextures(aiMaterial *mat, aiTextureType type);
};

#endif