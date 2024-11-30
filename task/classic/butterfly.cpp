#include "butterfly.hpp"
#include "glm/gtx/transform.hpp"
#include "error.hpp"

GL_TASK::Butterfly::Butterfly(const std::string &model_path, std::vector<Mesh *> &meshes, std::vector<MeshInstance *> &meshInstances, std::vector<Texture *> &textures, std::vector<Material> &materials)
    : danceAnimation(model_path), animator(&danceAnimation, translate_rand, rotate_rand), meshes(meshes), meshInstances(meshInstances), textures(textures), materials(materials)
{
    set_model_matrix();
    update_matrix();
    add_model(model_path);
}

// 添加模型
bool GL_TASK::Butterfly::add_model(const std::string &modelfilePath)
{
    Model *model = new Model();
    if (model->LoadFromFile(modelfilePath))
    {
        // 1. 将model中的纹理数据导入scene中
        int textureStartId = textures.size();
        for (auto texture : model->getTextures())
            textures.push_back(texture);

        // 2. 将左翅膀网格数据导入scene中
        start_meshInstance_id_l = meshInstances.size();
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
        end_meshInstance_id_l = meshInstances.size();

        // 3. 将右翅膀网格数据导入scene中
        start_meshInstance_id_r = meshInstances.size();
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
        end_meshInstance_id_r = meshInstances.size();
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

void GL_TASK::Butterfly::set_model_matrix()
{
    model_matrix = glm::mat4(1.0f);
    model_matrix = glm::scale(model_matrix, glm::vec3(1.f, 1.f, 1.f) * scale_rand);                   // 缩放
    model_matrix = glm::rotate(model_matrix, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));      // 正向
    model_matrix = glm::rotate(model_matrix, glm::radians(rotate_rand), glm::vec3(0.0f, 1.0f, 0.0f)); // 旋转
    model_matrix = glm::translate(model_matrix, glm::vec3(translate_rand, 0.0f, translate_rand));     // 平移
}

void GL_TASK::Butterfly::update_matrix()
{
    // float currentFrame = glfwGetTime() + differ;
    // deltaTime = currentFrame - lastFrame;
    // lastFrame = currentFrame;
    animator.UpdateAnimation(1 / 30.);

    keyframe_transforms_r = model_matrix * animator.m_KeyframeTransforms["R"];
    keyframe_transforms_l = model_matrix * animator.m_KeyframeTransforms["L"];
}

void GL_TASK::Butterfly::update()
{
    update_matrix();
    printf("keyframe_transforms_l: %f %f %f\n", keyframe_transforms_l[3][0], keyframe_transforms_l[3][1], keyframe_transforms_l[3][2]);
    for (int i = start_meshInstance_id_l; i < end_meshInstance_id_l; i++)
        meshInstances[i]->transform = keyframe_transforms_l;

    for (int i = start_meshInstance_id_r; i < end_meshInstance_id_r; i++)
        meshInstances[i]->transform = keyframe_transforms_r;
}
