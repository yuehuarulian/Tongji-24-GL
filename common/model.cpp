#include "model.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Model::Model(string const &path, bool gamma) : gammaCorrection(gamma)
{
    printf("start load model: %s\n", path.c_str());
    loadModel(path);
    printf("end load model: %s\n", path.c_str());
}

bool Model::loadModel(string const &path)
{
    //
    // 加载模型文件(.obj)
    // 使用Assimp库进行加载
    //
    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

    // 检查是否加载成功
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
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
    vector<Vertex> vertices;      // 顶点数据
    vector<unsigned int> indices; // 索引数据
    vector<Texture> textures;     // 纹理数据

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

            vector.x = mesh->mTangents[i].x;
            vector.y = mesh->mTangents[i].y;
            vector.z = mesh->mTangents[i].z;
            vertex.Tangent = vector; // 切线
            vector.x = mesh->mBitangents[i].x;
            vector.y = mesh->mBitangents[i].y;
            vector.z = mesh->mBitangents[i].z;
            vertex.Bitangent = vector; // 副切线
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
    m_material.baseColor = glm::vec3(0.0, 1.0, 0.0);
    aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
    {
        // 1. 漫反射纹理
        m_material.diffuseTexId = loadMaterialTextures(material, aiTextureType_DIFFUSE);
        // 2. 镜面反射纹理
        m_material.specularTexId = loadMaterialTextures(material, aiTextureType_SPECULAR);   
        // 3. 法线贴图
        m_material.heightTexId = loadMaterialTextures(material, aiTextureType_HEIGHT);     
        // 4. 高度贴图
        m_material.ambientTexId = loadMaterialTextures(material, aiTextureType_AMBIENT);    
        // PBR 相关贴图
        // 5. 金属度贴图
        m_material.metalnessTexId = loadMaterialTextures(material, aiTextureType_METALNESS);    
        // 6. 粗糙度贴图
        m_material.diffuse_roughnessTexId = loadMaterialTextures(material, aiTextureType_DIFFUSE_ROUGHNESS); 
        // 7. 环境光遮蔽贴图
        m_material.ambient_occlusionTexId = loadMaterialTextures(material, aiTextureType_AMBIENT_OCCLUSION);
       
        printf("/******************************/\n");
        printf("Material Texture ID INFO:\n");
        printf("diffuseTexId: #%f\n", m_material.diffuseTexId);
        printf("specularTexId: #%f\n", m_material.specularTexId);
        printf("heightTexId: #%f\n", m_material.heightTexId);
        printf("ambientTexId: #%f\n", m_material.ambientTexId);
        printf("metalnessTexId: #%f\n", m_material.metalnessTexId);
        printf("diffuse_roughnessTexId: #%f\n", m_material.diffuse_roughnessTexId);
        printf("ambient_occlusionTexId: #%f\n", m_material.ambient_occlusionTexId);
        printf("/******************************/\n");
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
    mat->GetTexture(type, 0, &str);                          // 一种类型的纹理推图只获取第一个
    string filename = directory + '/' + string(str.C_Str()); // 纹理文件路径

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
