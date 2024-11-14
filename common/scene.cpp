#include <scene.hpp>
#include <mesh.hpp>
#include <model.hpp>

bool Scene::AddModel(const std::string &modelfilePath)
{
    Model *model = new Model;
    if (model->LoadFromFile(modelfilePath))
    {
        for (auto mesh : model->getMeshes())
        {
            this->meshes.push_back(mesh);
            int mesh_id = meshes.size();
            glm::mat4 transformMat(1.0f);
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
void Scene::createTLAS()
{
}