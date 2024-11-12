#include <scene.hpp>
#include <mesh.hpp>
#include <model.hpp>

bool Scene::AddMesh(const std::string &filePath)
{
    std::cout << "Begin Load Mesh: " << filePath << std::endl;
    Model *model = new Model;
    if (model->LoadFromFile(filePath))
    {
        for (auto mesh : model->getMeshes())
            meshes.push_back(mesh);
    }
    else
    {
        std::cout << "ERROR::UNABLE TO LOAD MESH::" << filePath << std::endl;
        delete model;
        return false;
    }

    delete model;
    std::cout << "Mesh Load Success !" << std::endl;
    return true;
}

bool Scene::AddTexture(const std::string &filename)
{
    return true;
}