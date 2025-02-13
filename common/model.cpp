#include "model.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

unsigned int TextureFromFile(const char *path, const std::string &directory, bool gamma = false)
{
    std::string filename = std::string(path);
    filename = directory + '/' + filename;

    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);

    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        std::cout << "Texture successful to loaded at path: " << path << std::endl;
        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

Model::Model(std::string const &path, bool gamma) : gammaCorrection(gamma)
{
    loadModel(path);
}

bool Model::loadModel(std::string const &path)
{
    //
    // 加载模型文件(.obj)
    // 使用Assimp库进行加载
    //
    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs);

    // 检查是否加载成功
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
        return false;
    }
    // 提取文件路径的目录部分，用于后续加载纹理
    directory = path.substr(0, path.find_last_of('/'));

    // 递归处理 Assimp 的根节点及其子节点
    processNode(scene->mRootNode, scene);

    return true;
}

void Model::processNode(aiNode *node, const aiScene *scene)
{
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        Mesh *m_mesh = processMesh(mesh, scene); // 创建一个新的
        meshes.push_back(m_mesh);
    }
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processNode(node->mChildren[i], scene);
    }
}

// 将Assimp的Mesh数据类型转换为自定义的Mesh数据类型
Mesh *Model::processMesh(aiMesh *mesh, const aiScene *scene)
{
    std::vector<Vertex> vertices;      // 顶点数据
    std::vector<unsigned int> indices; // 索引数据
    std::vector<Texture> textures;     // 纹理数据

    // 1. 遍历所有的点 获取所有的顶点数据
    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex;
        glm::vec3 vector;

        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        vertex.Position = vector; // 顶点坐标

        if (mesh->HasNormals())
        {
            vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;
            vertex.Normal = vector; // 法线坐标
        }

        // 纹理坐标
        if (mesh->mTextureCoords[0])
        {
            glm::vec2 vec;
            // a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't
            // use models where a vertex can have multiple texture coordinates so we always take the first set (0).
            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.TexCoords = vec;
            // 删除切线和副切线
        }
        else
            vertex.TexCoords = glm::vec2(0.0f, 0.0f);

        vertices.push_back(vertex); // 将顶点添加到顶点数组
    }

    // 2. 遍历所有的面 获取所有的索引数据
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }

    // 3. 获取材质
    Material m_material = Material();
    aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
    printf("## The Material Params Number: %d ##\n", loadMaterialParams(m_material, material));
    // 4. 获取纹理贴图
    {
        // 1. 漫反射纹理
        m_material.diffuseTexId = loadMaterialTextures(material, aiTextureType_DIFFUSE);
        // 2. 镜面反射纹理
        m_material.specularTexId = loadMaterialTextures(material, aiTextureType_SPECULAR);
        // 3. 法线贴图
        m_material.normalTexId = loadMaterialTextures(material, aiTextureType_HEIGHT);
        // 4. 高度贴图
        m_material.heightTexId = loadMaterialTextures(material, aiTextureType_HEIGHT);
        // PBR 相关贴图
        // 5. 金属度贴图
        m_material.metalnessTexId = loadMaterialTextures(material, aiTextureType_METALNESS);
        // 6. 粗糙度贴图
        m_material.diffuse_roughnessTexId = loadMaterialTextures(material, aiTextureType_DIFFUSE_ROUGHNESS);
        // 7. 环境光遮蔽贴图
        m_material.ambient_occlusionTexId = loadMaterialTextures(material, aiTextureType_AMBIENT_OCCLUSION);
    }

    // 使用提取的数据创建并返回 Mesh 对象
    return new Mesh(vertices, indices, m_material);
}

int Model::loadMaterialTextures(aiMaterial *mat, aiTextureType type)
{
    // 返回一个纹理的索引
    int id = -1;

    int numOfTextures = mat->GetTextureCount(type); // 获取该种类型

    if (numOfTextures == 0)
        return id;

    aiString str;
    mat->GetTexture(type, 0, &str);                                    // 一种类型的纹理推图只获取第一个
    std::string filename = directory + '/' + std::string(str.C_Str()); // 纹理文件路径

    for (int i = 0; i < textures_loaded.size(); i++)
        if (textures_loaded[i]->texName == filename)
            return i;

    Texture *texture = new Texture();
    if (texture->LoadTexture(filename))
    {
        id = textures_loaded.size();        // id为对应的索引
        textures_loaded.push_back(texture); // 将纹理存储起来以便复用
    }
    else
    {
        printf("Unable to load texture %s\n", filename.c_str());
        delete texture;
        id = -1;
    }

    return id;
}

int Model::loadMaterialParams(Material& m_material, aiMaterial *mat)
{
    int loadnum = 0;
    // 提取颜色和物理属性
    aiColor3D color;
    float value;

    // 漫反射颜色 (Kd)
    if (mat->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS) {
        m_material.baseColor = glm::vec3(color.r, color.g, color.b);
        loadnum++;
    }

    // 镜面反射颜色 (Ks)
    if (mat->Get(AI_MATKEY_COLOR_SPECULAR, color) == AI_SUCCESS) {
        m_material.specularColor = glm::vec3(color.r, color.g, color.b);
        loadnum++;
    }

    // 自发光颜色 (Ke)
    if (mat->Get(AI_MATKEY_COLOR_EMISSIVE, color) == AI_SUCCESS) {
        m_material.emissiveColor = glm::vec3(color.r, color.g, color.b);
        loadnum++;
    }

    // 折射率 (Ni)
    if (mat->Get(AI_MATKEY_REFRACTI, value) == AI_SUCCESS) {
        m_material.refractionIndex = value;
        loadnum++;
    }

    // 透明度 (d)
    if (mat->Get(AI_MATKEY_OPACITY, value) == AI_SUCCESS) {
        m_material.transparency = value;
        loadnum++;
    }

    // 光照模型 (illum)
    int intValue;
    if (mat->Get(AI_MATKEY_SHADING_MODEL, intValue) == AI_SUCCESS) {
        m_material.illuminationModel = intValue;
        loadnum++;
    }

    // TODO::其他属性 (粗糙度、金属度等)
    // for (unsigned int i = 0; i < mat->mNumProperties; i++) {
    //     aiMaterialProperty *prop = mat->mProperties[i];
    //     std::cout << "Property Key: " << prop->mKey.C_Str() << " | Type: " << prop->mType << " | Size: " << prop->mDataLength << std::endl;
    // }
    if (mat->Get("$mat.roughnessFactor", 0, 0, value) == AI_SUCCESS) {
        m_material.roughness = value; // 粗糙度 (Pr)
        loadnum++;
    }
    if (mat->Get("$mat.metallicFactor", 0, 0, value) == AI_SUCCESS) {
        m_material.metalness = value; // 金属度 (Pm)
        loadnum++;
    }
    if (mat->Get("$mat.anisotropyFactor", 0, 0, value) == AI_SUCCESS) {
        m_material.scattering = value; // 金属度 (Pm)
        loadnum++;
    }
    if (mat->Get("$mat.clearcoat.factor", 0, 0, value) == AI_SUCCESS) {
        m_material.coating = value; // 涂层 (Pc)
        loadnum++;
    }
    if (mat->Get("$mat.clearcoat.roughnessFactor", 0, 0, value) == AI_SUCCESS) {
        m_material.coatRoughness = value; // 涂层粗糙度 (Pcr)
        loadnum++;
    }

    return loadnum;
}