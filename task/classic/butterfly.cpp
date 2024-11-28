#include "butterfly.hpp"
#include "glm/gtx/transform.hpp"
#include "error.hpp"

GL_TASK::Butterfly::Butterfly(const std::string &model_path) : danceAnimation(model_path), animator(&danceAnimation, translate_rand, rotate_rand) {}

// 添加模型
bool GL_TASK::Butterfly::add_model(const std::string &modelfilePath, std::vector<Mesh *> &meshes, std::vector<MeshInstance *> &meshInstances,
                                   std::vector<Texture *> &textures, std::vector<Material> &materials)
{
    update();
    Model *model = new Model();
    if (model->LoadFromFile(modelfilePath))
    {
        // 1. 将model中的纹理数据导入scene中
        int textureStartId = textures.size();
        for (auto texture : model->getTextures())
            textures.push_back(texture);

        // 2. 将左翅膀网格数据导入scene中
        for (auto mesh : model->getMeshes())
        {
            int mesh_id = meshes.size();
            int materialStartId = materials.size();

            mesh->material.updateTexId(textureStartId);
            materials.push_back(mesh->material);
            MeshInstance *instance = new MeshInstance(mesh_id, materialStartId, keyframe_transforms_l);

            meshes.push_back(mesh);
            meshInstances.push_back(instance);
        }

        // 3. 将右翅膀网格数据导入scene中
        for (auto mesh : model->getMeshes())
        {
            int mesh_id = meshes.size();
            int materialStartId = materials.size();

            mesh->material.updateTexId(textureStartId);
            materials.push_back(mesh->material);
            MeshInstance *instance = new MeshInstance(mesh_id, materialStartId, keyframe_transforms_r);

            meshes.push_back(mesh);
            meshInstances.push_back(instance);
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

void GL_TASK::Butterfly::update_matrix()
{
    float currentFrame = glfwGetTime() + differ;
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
    animator.UpdateAnimation(1 / 30.);

    keyframe_transforms_r = model_matrix * animator.m_KeyframeTransforms["R"];
    keyframe_transforms_l = model_matrix * animator.m_KeyframeTransforms["L"];
}

void GL_TASK::Butterfly::update()
{
}