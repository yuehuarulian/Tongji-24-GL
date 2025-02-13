#include <scene.hpp>
#include <mesh.hpp>
#include <model.hpp>
#include <iostream>
#include <vector>
#include <sstream>
#include <iomanip>
#include <filesystem>

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize.h"
#include "stb_image.h"
#include "stb_image_write.h"

// 添加模型
bool Scene::add_model(const std::string &modelfilePath, glm::mat4 transformMat)
{
    std::shared_ptr<Model> model = std::make_shared<Model>();
    if (model->LoadFromFile(modelfilePath))
    {
        // 1. 将model中的纹理数据导入scene中
        int textureStartId = this->textures.size();
        for (auto texture : model->getTextures())
            this->textures.push_back(texture);
        printf("Add Textures\n");

        // 2. 将model中的网格数据导入scene中
        for (auto mesh : model->getMeshes())
        {
            int mesh_id = meshes.size();
            int materialStartId = this->materials.size();

            // 3. 将mesh中的材质信息导入scene中
            //    同时更新纹理索引
            mesh->material.updateTexId(textureStartId);
            this->materials.push_back(mesh->material);
            // 4. 根据网格id和材质id创建一个meshInstance
            MeshInstance *instance = new MeshInstance(mesh_id, materialStartId, transformMat);

            this->meshes.push_back(mesh);
            this->meshInstances.push_back(instance);
        }
        printf("Add Mesh Data\n");
    }
    else
    {
        std::cout << "ERROR::UNABLE TO LOAD MESH::" << modelfilePath << std::endl;
        return false;
    }

    return true;
}

// 添加材质
int Scene::add_material(const Material &material)
{
    int id = materials.size();
    materials.push_back(material);
    return id;
}

// 添加纹理
int Scene::add_texture(const std::string &filename)
{
    // 根据纹理路径的名称 返回其在纹理数组中的索引位置
    int id = -1;
    for (int i = 0; i < textures.size(); i++)
    {
        if (textures[i]->texName == filename)
        {
            return i;
        }
    }

    id = textures.size();
    Texture *texture = new Texture();

    printf("Loading Texture %s\n", filename.c_str());
    if (texture->LoadTexture(filename))
    {
        textures.push_back(texture);
    }
    else
    {
        printf("Unable to load texture %s\n", filename.c_str());
        delete texture;
        id = -1;
    }

    return id;
}

// 创建底层加速结构 -- 以每个Mesh中的三角形作为基本单元
void Scene::createBLAS()
{
#pragma omp parallel for
    for (int i = 0; i < meshes.size(); i++)
    {
        meshes[i]->BuildBVH();
    }
}

// 创建顶层加速结构 -- 与场景中的Mesh作为基本单元
void Scene::createTLAS()
{
    std::vector<AABB> bounds;
    bounds.resize(meshInstances.size());
    int step = 0;
    for (int i = 0; i < meshInstances.size(); i++)
    {
        AABB bbox = meshes[meshInstances[i]->meshID]->bvh->getBounds(); // 获取Mesh实例对应的包围盒
        glm::mat4 matrix = meshInstances[i]->transform;                 // 获取Mesh实例的空间变化矩阵Model

        glm::vec3 minBound = bbox.getMinP();
        glm::vec3 maxBound = bbox.getMaxP();

        glm::vec3 right = glm::vec3(matrix[0][0], matrix[0][1], matrix[0][2]);
        glm::vec3 up = glm::vec3(matrix[1][0], matrix[1][1], matrix[1][2]);
        glm::vec3 forward = glm::vec3(matrix[2][0], matrix[2][1], matrix[2][2]);
        glm::vec3 translation = glm::vec3(matrix[3][0], matrix[3][1], matrix[3][2]);

        glm::vec3 xa = right * minBound.x;
        glm::vec3 xb = right * maxBound.x;

        glm::vec3 ya = up * minBound.y;
        glm::vec3 yb = up * maxBound.y;

        glm::vec3 za = forward * minBound.z;
        glm::vec3 zb = forward * maxBound.z;

        minBound = glm::min(xa, xb) + glm::min(ya, yb) + glm::min(za, zb) + translation;
        maxBound = glm::max(xa, xb) + glm::max(ya, yb) + glm::max(za, zb) + translation;

        AABB bound(minBound, maxBound);
        bounds[i] = bound;
    }
    printf("\n*****************\n");
    printf("SCENE TOP MESH BVH:\n");
    if (sceneBVH)
    {
        delete sceneBVH;
        sceneBVH = new BVH(10.0f, 64, false);
    }
    sceneBVH->Build(&bounds[0], bounds.size());
    sceneBVH->PrintStatistics(std::cout);
    printf("\n*****************\n");
    sceneBounds = sceneBVH->getBounds();
}

// 处理数据
void Scene::process_data()
{
    vertIndices.clear();
    verticesUVX.clear();
    normalsUVY.clear();
    transforms.clear();
    textureMapsArray.clear();
    // 将BVH的顶层节点和底层节点转换为适合传递给GPU的节点
    // bvhConverter.nodes即我们所需要的数据
    bvhConverter.Process(sceneBVH, meshes, meshInstances);

    // 随后将顶点数据、法线数据、UV纹理坐标数据转换为适合传递给GPU的数据
    int verticesCnt = 0;
    vertIndices.clear();
    verticesUVX.clear();
    normalsUVY.clear();
    for (const auto &mesh : meshes) // 遍历所有的网格
    {
        // 获取 BVH 的索引数量和索引数据
        int numIndices = mesh->bvh->getNumIndices();     // 索引的数量
        const int *triIndices = mesh->bvh->getIndices(); // 获取索引 -- 每个存放的是第几个三角形

        // 复制索引数据，处理共享三角形的情况
        for (int j = 0; j < numIndices; j++)
        {
            int index = triIndices[j]; // 第几个三角形
            vertIndices.push_back(Indices{
                mesh->indices[index * 3 + 0] + verticesCnt,
                mesh->indices[index * 3 + 1] + verticesCnt,
                mesh->indices[index * 3 + 2] + verticesCnt});
        }

        std::vector<glm::vec4> m_verticesUVX;
        std::vector<glm::vec4> m_normalsUVY;

        mesh->ProcessVertices(m_verticesUVX, m_normalsUVY);

        // 合并顶点和法线数据
        verticesUVX.insert(verticesUVX.end(), m_verticesUVX.begin(), m_verticesUVX.end());
        normalsUVY.insert(normalsUVY.end(), m_normalsUVY.begin(), m_normalsUVY.end());

        // 更新顶点计数
        verticesCnt += mesh->vertices.size();
    }

    // 变换矩阵
    transforms.resize(meshInstances.size());
    for (int i = 0; i < meshInstances.size(); i++)
        transforms[i] = meshInstances[i]->transform;

    const int texBytes = texArrayHeight * texArrayWidth * 4;
    textureMapsArray.resize(texBytes * textures.size());

#pragma omp parallel for
    for (int i = 0; i < textures.size(); i++)
    {
        int texWidth = textures[i]->width;
        int texHeight = textures[i]->height;
        if (texWidth != texArrayWidth || texHeight != texArrayHeight)
        {
            unsigned char *resizedTex = new unsigned char[texBytes];
            stbir_resize_uint8(&textures[i]->texData[0], texWidth, texHeight, 0, resizedTex, texArrayHeight, texArrayWidth, 0, 4);
            std::copy(resizedTex, resizedTex + texBytes, &textureMapsArray[i * texBytes]);
            delete[] resizedTex;
        }
        else
            std::copy(textures[i]->texData.begin(), textures[i]->texData.end(), &textureMapsArray[i * texBytes]);
    }
}

// 将着色器所需要的数据转换为纹理数据
void Scene::init_GPU_data()
{
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    // ---------- BVH树节点数据 ---------- //
    // 注释:
    // struct Node
    // {
    //     glm::vec3 bboxmin;
    //     glm::vec3 bboxmax;
    //     glm::vec3 LRLeaf;
    // };
    // 节点分为两部分 - 顶层的BVH节点和底层的BVH节点
    // 前面的部分是底层的BVH节点 后面的部分是顶层的BVH节点
    // 无论是底层的还是顶层的BVH节点都包含包围盒的最小点和最大点
    // 其中LRLeaf.z表示节点的类型 ( ==0表示Internal节点 >0表示底层Leaf节点 <0表示顶层Leaf节点)
    // 对于Internal节点来说 LRLeaf.x表示左孩子节点 LRLeaf.y表示右孩子节点
    // 对于底层Leaf节点(Mesh)来说 LRLeaf.x表示该mesh起点 LRLeaf.y表示包含的三角形数量
    // 对于顶层Leaf节点(MeshInstance)来说 LRLeaf.x表示所代表mesh起点 LRLeaf.y表示材质ID
    glGenBuffers(1, &BVHBuffer);
    glBindBuffer(GL_TEXTURE_BUFFER, BVHBuffer);
    BVHBufferSize = sizeof(BVHConverter::Node) * bvhConverter.nodes.size();
    glBufferData(GL_TEXTURE_BUFFER, sizeof(BVHConverter::Node) * bvhConverter.nodes.size(), &bvhConverter.nodes[0], GL_STATIC_DRAW);
    glGenTextures(1, &BVHTex);
    glBindTexture(GL_TEXTURE_BUFFER, BVHTex);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGB32F, BVHBuffer);
    // ---------- 顶点索引数据 ---------- //
    // 注释:
    glGenBuffers(1, &vertexIndicesBuffer);
    glBindBuffer(GL_TEXTURE_BUFFER, vertexIndicesBuffer);
    vertexIndicesBufferSize = sizeof(Indices) * vertIndices.size();
    glBufferData(GL_TEXTURE_BUFFER, sizeof(Indices) * vertIndices.size(), &vertIndices[0], GL_STATIC_DRAW);
    glGenTextures(1, &vertexIndicesTex);
    glBindTexture(GL_TEXTURE_BUFFER, vertexIndicesTex);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGB32I, vertexIndicesBuffer);
    // ---------- 顶点/UV数据 ---------- //
    // 注释:
    glGenBuffers(1, &verticesBuffer);
    glBindBuffer(GL_TEXTURE_BUFFER, verticesBuffer);
    verticesBufferSize = sizeof(glm::vec4) * verticesUVX.size();
    glBufferData(GL_TEXTURE_BUFFER, sizeof(glm::vec4) * verticesUVX.size(), &verticesUVX[0], GL_STATIC_DRAW);
    glGenTextures(1, &verticesTex);
    glBindTexture(GL_TEXTURE_BUFFER, verticesTex);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, verticesBuffer);
    // ---------- 法线/UV数据 ---------- //
    // 注释:
    glGenBuffers(1, &normalsBuffer);
    glBindBuffer(GL_TEXTURE_BUFFER, normalsBuffer);
    normalsBufferSize = sizeof(glm::vec4) * normalsUVY.size();
    glBufferData(GL_TEXTURE_BUFFER, sizeof(glm::vec4) * normalsUVY.size(), &normalsUVY[0], GL_STATIC_DRAW);
    glGenTextures(1, &normalsTex);
    glBindTexture(GL_TEXTURE_BUFFER, normalsTex);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, normalsBuffer);
    // ---------- 转换矩阵数据 ---------- //
    // 注释:
    glGenBuffers(1, &transformsBuffer);
    glBindBuffer(GL_TEXTURE_BUFFER, transformsBuffer);
    transformsBufferSize = sizeof(glm::mat4) * transforms.size();
    glBufferData(GL_TEXTURE_BUFFER, sizeof(glm::mat4) * transforms.size(), &transforms[0], GL_STATIC_DRAW);
    glGenTextures(1, &transformsTex);
    glBindTexture(GL_TEXTURE_BUFFER, transformsTex);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, transformsBuffer);
    // ---------- 材质数据 ---------- //
    // 注释：
    glGenBuffers(1, &materialsBuffer);
    glBindBuffer(GL_TEXTURE_BUFFER, materialsBuffer);
    materialsBufferSize = sizeof(Material) * materials.size();
    glBufferData(GL_TEXTURE_BUFFER, sizeof(Material) * materials.size(), &materials[0], GL_STATIC_DRAW);
    glGenTextures(1, &materialsTex);
    glBindTexture(GL_TEXTURE_BUFFER, materialsTex);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, materialsBuffer);
    // ---------- 纹理数据 ---------- //
    // 注释：
    if (!textures.empty())
    {
        glGenTextures(1, &textureMapsArrayTex);
        glBindTexture(GL_TEXTURE_2D_ARRAY, textureMapsArrayTex);
        glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA8, texArrayWidth, texArrayHeight, textures.size(), 0, GL_RGBA, GL_UNSIGNED_BYTE, &textureMapsArray[0]);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    }

    // 绑定纹理数据
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_BUFFER, BVHTex);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_BUFFER, vertexIndicesTex);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_BUFFER, verticesTex);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_BUFFER, normalsTex);
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_BUFFER, transformsTex);
    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_BUFFER, materialsTex);
    glActiveTexture(GL_TEXTURE7);
    glBindTexture(GL_TEXTURE_2D_ARRAY, textureMapsArrayTex);
}

void Scene::update_GPU_data()
{
    // 更新 BVH 数据
    if (BVHBuffer)
    {
        glDeleteBuffers(1, &BVHBuffer);
        glDeleteTextures(1, &BVHTex);
    }
    glGenBuffers(1, &BVHBuffer);
    glBindBuffer(GL_TEXTURE_BUFFER, BVHBuffer);
    glBufferData(GL_TEXTURE_BUFFER, sizeof(BVHConverter::Node) * bvhConverter.nodes.size(), &bvhConverter.nodes[0], GL_STATIC_DRAW);
    glGenTextures(1, &BVHTex);
    glBindTexture(GL_TEXTURE_BUFFER, BVHTex);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGB32F, BVHBuffer);

    // 更新顶点索引数据
    if (vertexIndicesBuffer)
    {
        glDeleteBuffers(1, &vertexIndicesBuffer);
        glDeleteTextures(1, &vertexIndicesTex);
    }
    glGenBuffers(1, &vertexIndicesBuffer);
    glBindBuffer(GL_TEXTURE_BUFFER, vertexIndicesBuffer);
    glBufferData(GL_TEXTURE_BUFFER, sizeof(Indices) * vertIndices.size(), &vertIndices[0], GL_STATIC_DRAW);
    glGenTextures(1, &vertexIndicesTex);
    glBindTexture(GL_TEXTURE_BUFFER, vertexIndicesTex);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGB32I, vertexIndicesBuffer);

    // 更新顶点/UV数据
    if (verticesBuffer)
    {
        glDeleteBuffers(1, &verticesBuffer);
        glDeleteTextures(1, &verticesTex);
    }
    glGenBuffers(1, &verticesBuffer);
    glBindBuffer(GL_TEXTURE_BUFFER, verticesBuffer);
    glBufferData(GL_TEXTURE_BUFFER, sizeof(glm::vec4) * verticesUVX.size(), &verticesUVX[0], GL_STATIC_DRAW);
    glGenTextures(1, &verticesTex);
    glBindTexture(GL_TEXTURE_BUFFER, verticesTex);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, verticesBuffer);

    // 更新法线/UV数据
    if (normalsBuffer)
    {
        glDeleteBuffers(1, &normalsBuffer);
        glDeleteTextures(1, &normalsTex);
    }
    glGenBuffers(1, &normalsBuffer);
    glBindBuffer(GL_TEXTURE_BUFFER, normalsBuffer);
    glBufferData(GL_TEXTURE_BUFFER, sizeof(glm::vec4) * normalsUVY.size(), &normalsUVY[0], GL_STATIC_DRAW);
    glGenTextures(1, &normalsTex);
    glBindTexture(GL_TEXTURE_BUFFER, normalsTex);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, normalsBuffer);

    // 更新转换矩阵数据
    glBindBuffer(GL_TEXTURE_BUFFER, transformsBuffer);
    glBufferSubData(GL_TEXTURE_BUFFER, 0, sizeof(glm::mat4) * transforms.size(), &transforms[0]);
    glBindTexture(GL_TEXTURE_BUFFER, transformsTex);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, transformsBuffer);

    // 更新材质数据
    glBindBuffer(GL_TEXTURE_BUFFER, materialsBuffer);
    glBufferSubData(GL_TEXTURE_BUFFER, 0, sizeof(Material) * materials.size(), &materials[0]);
    glBindTexture(GL_TEXTURE_BUFFER, materialsTex);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, materialsBuffer);

    // 更新纹理数据
    if (!textures.empty())
    {
        glBindTexture(GL_TEXTURE_2D_ARRAY, textureMapsArrayTex);
        glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA8, texArrayWidth, texArrayHeight, textures.size(), 0, GL_RGBA, GL_UNSIGNED_BYTE, &textureMapsArray[0]);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    }

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_BUFFER, BVHTex);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_BUFFER, vertexIndicesTex);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_BUFFER, verticesTex);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_BUFFER, normalsTex);
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_BUFFER, transformsTex);
    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_BUFFER, materialsTex);
    glActiveTexture(GL_TEXTURE7);
    glBindTexture(GL_TEXTURE_2D_ARRAY, textureMapsArrayTex);
}

void Scene::update_GPU_data1()
{
    glPixelStorei(GL_PACK_ALIGNMENT, 1);

    size_t newSize = 0;
    // ---------- 更新 BVH树节点数据 ---------- //
    newSize = sizeof(BVHConverter::Node) * bvhConverter.nodes.size();
    if (newSize != BVHBufferSize)
    {
        // 重新分配缓冲区
        glBindBuffer(GL_TEXTURE_BUFFER, BVHBuffer);
        glBufferData(GL_TEXTURE_BUFFER, newSize, &bvhConverter.nodes[0], GL_DYNAMIC_DRAW);
        std::cerr << "BVH node buffer size changed from " << BVHBufferSize << " to " << newSize << std::endl;
        BVHBufferSize = newSize;
    }
    else
    {
        // 仅更新数据
        glBindBuffer(GL_TEXTURE_BUFFER, BVHBuffer);
        glBufferSubData(GL_TEXTURE_BUFFER, 0, newSize, &bvhConverter.nodes[0]);
    }
    glBindTexture(GL_TEXTURE_BUFFER, BVHTex);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGB32F, BVHBuffer);

    // ---------- 更新顶点索引数据 ---------- //
    newSize = sizeof(Indices) * vertIndices.size();
    if (newSize != vertexIndicesBufferSize)
    {
        glBindBuffer(GL_TEXTURE_BUFFER, vertexIndicesBuffer);
        glBufferData(GL_TEXTURE_BUFFER, newSize, &vertIndices[0], GL_DYNAMIC_DRAW);
        std::cerr << "Vertex index buffer size changed from " << vertexIndicesBufferSize << " to " << newSize << std::endl;
        vertexIndicesBufferSize = newSize;
    }
    else
    {
        glBindBuffer(GL_TEXTURE_BUFFER, vertexIndicesBuffer);
        glBufferSubData(GL_TEXTURE_BUFFER, 0, newSize, &vertIndices[0]);
    }
    glBindTexture(GL_TEXTURE_BUFFER, vertexIndicesTex);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGB32I, vertexIndicesBuffer);

    // ---------- 更新顶点/UV数据 ---------- //
    newSize = sizeof(glm::vec4) * verticesUVX.size();
    if (newSize != verticesBufferSize)
    {
        glBindBuffer(GL_TEXTURE_BUFFER, verticesBuffer);
        glBufferData(GL_TEXTURE_BUFFER, newSize, &verticesUVX[0], GL_DYNAMIC_DRAW);
        std::cerr << "Vertex buffer size changed from " << verticesBufferSize << " to " << newSize << std::endl;
        verticesBufferSize = newSize;
    }
    else
    {
        glBindBuffer(GL_TEXTURE_BUFFER, verticesBuffer);
        glBufferSubData(GL_TEXTURE_BUFFER, 0, newSize, &verticesUVX[0]);
    }
    glBindTexture(GL_TEXTURE_BUFFER, verticesTex);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, verticesBuffer);

    // ---------- 更新法线/UV数据 ---------- //
    newSize = sizeof(glm::vec4) * normalsUVY.size();
    if (newSize != normalsBufferSize)
    {
        glBindBuffer(GL_TEXTURE_BUFFER, normalsBuffer);
        glBufferData(GL_TEXTURE_BUFFER, newSize, &normalsUVY[0], GL_DYNAMIC_DRAW);
        std::cerr << "Normal buffer size changed from " << normalsBufferSize << " to " << newSize << std::endl;
        normalsBufferSize = newSize;
    }
    else
    {
        glBindBuffer(GL_TEXTURE_BUFFER, normalsBuffer);
        glBufferSubData(GL_TEXTURE_BUFFER, 0, newSize, &normalsUVY[0]);
    }
    glBindTexture(GL_TEXTURE_BUFFER, normalsTex);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, normalsBuffer);

    // ---------- 更新转换矩阵数据 ---------- //
    newSize = sizeof(glm::mat4) * transforms.size();
    if (newSize != transformsBufferSize)
    {
        glBindBuffer(GL_TEXTURE_BUFFER, transformsBuffer);
        glBufferData(GL_TEXTURE_BUFFER, newSize, &transforms[0], GL_DYNAMIC_DRAW);
        std::cerr << "Transform buffer size changed from " << transformsBufferSize << " to " << newSize << std::endl;
        transformsBufferSize = newSize;
    }
    else
    {
        glBindBuffer(GL_TEXTURE_BUFFER, transformsBuffer);
        glBufferSubData(GL_TEXTURE_BUFFER, 0, newSize, &transforms[0]);
    }
    glBindTexture(GL_TEXTURE_BUFFER, transformsTex);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, transformsBuffer);

    // ---------- 更新材质数据 ---------- //
    newSize = sizeof(Material) * materials.size();

    if (newSize != materialsBufferSize)
    {
        glBindBuffer(GL_TEXTURE_BUFFER, materialsBuffer);
        glBufferData(GL_TEXTURE_BUFFER, newSize, &materials[0], GL_DYNAMIC_DRAW);
        std::cerr << "Material buffer size changed from " << materialsBufferSize << " to " << newSize << std::endl;
        materialsBufferSize = newSize;
    }
    else
    {
        glBindBuffer(GL_TEXTURE_BUFFER, materialsBuffer);
        glBufferSubData(GL_TEXTURE_BUFFER, 0, newSize, &materials[0]);
    }
    glBindTexture(GL_TEXTURE_BUFFER, materialsTex);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, materialsBuffer);

    // ---------- 更新纹理数据 ---------- //
    if (!textures.empty())
    {
        glBindTexture(GL_TEXTURE_2D_ARRAY, textureMapsArrayTex);
        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, texArrayWidth, texArrayHeight, textures.size(), GL_RGBA, GL_UNSIGNED_BYTE, &textureMapsArray[0]);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    }
}

// 初始化帧缓冲数据
void Scene::init_FBOs()
{
    // Path tracing FBO
    glGenFramebuffers(1, &pathTraceFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, pathTraceFBO);

    glGenTextures(1, &pathTraceTexture);
    glBindTexture(GL_TEXTURE_2D, pathTraceTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pathTraceTexture, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cerr << "Path trace FBO is not complete!" << std::endl;

    // Accumulation FBO
    glGenFramebuffers(1, &accumFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, accumFBO);

    glGenTextures(1, &accumTexture);
    glBindTexture(GL_TEXTURE_2D, accumTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, accumTexture, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cerr << "Accumulation FBO is not complete!" << std::endl;

    // Output FBO
    glGenFramebuffers(1, &outputFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, outputFBO);

    glGenTextures(1, &outputTexture[0]);
    glBindTexture(GL_TEXTURE_2D, outputTexture[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glGenTextures(1, &outputTexture[1]);
    glBindTexture(GL_TEXTURE_2D, outputTexture[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGBA, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, outputTexture[0], 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cerr << "Output FBO is not complete!" << std::endl;

    // Clear accumulation buffer
    glBindFramebuffer(GL_FRAMEBUFFER, accumFBO);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // For Denoiser
    frameOutputPtr = new glm::vec3[WINDOW_WIDTH * WINDOW_HEIGHT];
}

void Scene::SaveFrameImage()
{
    glBindTexture(GL_TEXTURE_2D, outputTexture[1 - currentBuffer]);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, frameOutputPtr);
    std::vector<unsigned char> pixels(WINDOW_WIDTH * WINDOW_HEIGHT * 3);

    for (int i = 0; i < WINDOW_WIDTH * WINDOW_HEIGHT; ++i)
    {
        glm::vec3 color = frameOutputPtr[i];
        pixels[i * 3 + 0] = static_cast<unsigned char>(glm::clamp(color.r * 255.0f, 0.0f, 255.0f)); // Red
        pixels[i * 3 + 1] = static_cast<unsigned char>(glm::clamp(color.g * 255.0f, 0.0f, 255.0f)); // Green
        pixels[i * 3 + 2] = static_cast<unsigned char>(glm::clamp(color.b * 255.0f, 0.0f, 255.0f)); // Blue
    }

    std::ostringstream oss;
    oss << "./render_imgs/frame_" << std::setw(3) << std::setfill('0') << frameNum << ".png";
    std::cout << "Save Picture " << oss.str() << std::endl;
    std::cerr << "Save Picture " << oss.str() << std::endl;
    stbi_flip_vertically_on_write(true);
    stbi_write_png(oss.str().c_str(), WINDOW_WIDTH, WINDOW_HEIGHT, 3, pixels.data(), WINDOW_WIDTH * 3);
}

void Scene::update_FBOs()
{
    // 更新 Path tracing FBO
    glBindFramebuffer(GL_FRAMEBUFFER, pathTraceFBO);
    glBindTexture(GL_TEXTURE_2D, pathTraceTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pathTraceTexture, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cerr << "Path trace FBO is not complete!" << std::endl;

    // 更新 Accumulation FBO
    glBindFramebuffer(GL_FRAMEBUFFER, accumFBO);
    glBindTexture(GL_TEXTURE_2D, accumTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, accumTexture, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cerr << "Accumulation FBO is not complete!" << std::endl;

    // 更新 Output FBO
    glBindFramebuffer(GL_FRAMEBUFFER, outputFBO);
    glBindTexture(GL_TEXTURE_2D, outputTexture[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D, outputTexture[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGBA, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, outputTexture[0], 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cerr << "Output FBO is not complete!" << std::endl;

    glBindTexture(GL_TEXTURE_2D, outputTexture[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGBA, GL_FLOAT, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, outputTexture[1], 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cerr << "Output FBO is not complete!" << std::endl;

    // 清除累积缓冲区
    glBindFramebuffer(GL_FRAMEBUFFER, accumFBO);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

glm::vec3 *Scene::get_frame_output()
{
    // 获取当前帧的输出
    glBindTexture(GL_TEXTURE_2D, outputTexture[1 - currentBuffer]);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, frameOutputPtr);
    return frameOutputPtr;
}