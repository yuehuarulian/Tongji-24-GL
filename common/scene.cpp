#include <scene.hpp>
#include <mesh.hpp>
#include <model.hpp>

bool Scene::AddModel(const std::string &modelfilePath, glm::mat4 transformMat)
{
    Model *model = new Model;
    if (model->LoadFromFile(modelfilePath))
    {
        for (auto mesh : model->getMeshes())
        {
            int mesh_id = meshes.size();
            this->meshes.push_back(mesh);
            MeshInstance *instance = new MeshInstance(mesh_id, transformMat);
            this->meshInstances.push_back(instance);
        }
    }
    else
    {
        std::cout << "ERROR::UNABLE TO LOAD MESH::" << modelfilePath << std::endl;
        delete model;
        return false;
    }

    delete model;
    return true;
}

bool Scene::AddTexture(const std::string &filename)
{
    return true;
}

// 创建底层加速结构 -- 以每个Mesh中的三角形作为基本单元
void Scene::createBLAS()
{
#pragma omp parallel for
    for (int i = 0; i < meshes.size(); i++)
    {
        printf("\n*****************\n");
        printf("MESH #%d BVH INFO: \n", i);
        meshes[i]->BuildBVH();
        meshes[i]->bvh->PrintStatistics(std::cout);
        printf("\n*****************\n");
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
    printf("SCENE MESH BVH:\n");
    sceneBVH->Build(&bounds[0], bounds.size());
    sceneBVH->PrintStatistics(std::cout);
    printf("\n*****************\n");
    sceneBounds = sceneBVH->getBounds();
}

void Scene::copyMeshData()
{
    // Copy顶点等数据
    int verticesCnt = 0;
    for (const auto &mesh : meshes)
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

    // Copy变换矩阵
    transforms.resize(meshInstances.size());
    for (int i = 0; i < meshInstances.size(); i++)
        transforms[i] = meshInstances[i]->transform;
}

void Scene::InitGPUData()
{
    glPixelStorei(GL_PACK_ALIGNMENT, 1);

    glGenBuffers(1, &BVHBuffer);
    glBindBuffer(GL_TEXTURE_BUFFER, BVHBuffer);
    glBufferData(GL_TEXTURE_BUFFER, sizeof(BVHConverter::Node) * bvhConverter.nodes.size(), &bvhConverter.nodes[0], GL_STATIC_DRAW);
    glGenTextures(1, &BVHTex);
    glBindTexture(GL_TEXTURE_BUFFER, BVHTex);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGB32F, BVHBuffer);

    glGenBuffers(1, &vertexIndicesBuffer);
    glBindBuffer(GL_TEXTURE_BUFFER, vertexIndicesBuffer);
    glBufferData(GL_TEXTURE_BUFFER, sizeof(Indices) * vertIndices.size(), &vertIndices[0], GL_STATIC_DRAW);
    glGenTextures(1, &vertexIndicesTex);
    glBindTexture(GL_TEXTURE_BUFFER, vertexIndicesTex);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGB32I, vertexIndicesBuffer);

    glGenBuffers(1, &verticesBuffer);
    glBindBuffer(GL_TEXTURE_BUFFER, verticesBuffer);
    glBufferData(GL_TEXTURE_BUFFER, sizeof(glm::vec4) * verticesUVX.size(), &verticesUVX[0], GL_STATIC_DRAW);
    glGenTextures(1, &verticesTex);
    glBindTexture(GL_TEXTURE_BUFFER, verticesTex);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, verticesBuffer);

    glGenBuffers(1, &normalsBuffer);
    glBindBuffer(GL_TEXTURE_BUFFER, normalsBuffer);
    glBufferData(GL_TEXTURE_BUFFER, sizeof(glm::vec4) * normalsUVY.size(), &normalsUVY[0], GL_STATIC_DRAW);
    glGenTextures(1, &normalsTex);
    glBindTexture(GL_TEXTURE_BUFFER, normalsTex);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, normalsBuffer);

    glGenTextures(1, &transformsTex);
    glBindTexture(GL_TEXTURE_2D, transformsTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, (sizeof(glm::mat4) / sizeof(glm::vec4)) * transforms.size(), 1, 0, GL_RGBA, GL_FLOAT, &transforms[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);

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

    
}