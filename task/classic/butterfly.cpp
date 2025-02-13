#include "butterfly.hpp"
#include "glm/gtx/transform.hpp"
#include "config.hpp"

GL_TASK::Butterfly::Butterfly(const std::string &model_path, std::vector<Mesh *> &meshes, std::vector<MeshInstance *> &meshInstances, std::vector<Texture *> &textures, std::vector<Material> &materials, bool is_main_butterfly)
    : danceAnimation(model_path), animator(&danceAnimation, translate_rand, rotate_rand), RenderableModel(meshes, meshInstances, textures, materials), mainButterfly(is_main_butterfly)
{
    set_model_matrix();
    update_matrix();
    add_model(model_path);
}

void GL_TASK::Butterfly::update()
{
    update_matrix();
    for (int i = start_meshInstance_id_l; i < end_meshInstance_id_l; i++)
        meshInstances[i]->transform = keyframe_transforms_l;

    std::cout << "keyframe_transforms_r: " << glm::to_string(keyframe_transforms_r) << std::endl;

    for (int i = start_meshInstance_id_r; i < end_meshInstance_id_r; i++)
        meshInstances[i]->transform = keyframe_transforms_r;
}

// 添加模型
bool GL_TASK::Butterfly::add_model(const std::string &model_path)
{
    Model *model = new Model();
    if (model->LoadFromFile(model_path))
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
        std::cout << "ERROR::UNABLE TO LOAD MESH::" << model_path << std::endl;
        delete model;
        return false;
    }

    delete model;
    return true;
}

void GL_TASK::Butterfly::set_model_matrix()
{
    model_matrix = glm::mat4(1.0f);
    if (mainButterfly)
    {
        /*蝴蝶原地飞*/
        // model_matrix = glm::scale(model_matrix, glm::vec3(1.f, 1.f, 1.f) * 8.0f);
        // model_matrix = glm::translate(model_matrix, glm::vec3(0.0f, translate_rand-9.0f, translate_rand-20.0f));
        // model_matrix = glm::rotate(model_matrix, glm::radians(-10.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        /*蝴蝶贴水飞*/
        model_matrix = glm::scale(model_matrix, glm::vec3(1.f, 1.f, 1.f) * 4.0f);
        // model_matrix = glm::translate(model_matrix, glm::vec3(0.0f, translate_rand - 55.0f, translate_rand + 5.0f));
        model_matrix = glm::translate(model_matrix, glm::vec3(0.0f, -50.0f, 5.0f));
        model_matrix = glm::rotate(model_matrix, glm::radians(-10.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        /*犹豫*/ /*高光*/
        // model_matrix = glm::scale(model_matrix, glm::vec3(1.f, 1.f, 1.f) * 8.0f);
        // model_matrix = glm::translate(model_matrix, glm::vec3(0.0f, translate_rand - 10.0f, translate_rand - 5.0f));
        // model_matrix = glm::rotate(model_matrix, glm::radians(-10.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    }
    else
    {
        /*蝴蝶原地飞需要注释clasicscene中的蝴蝶群循环*/ /*蝴蝶贴水飞需要注释clasicscene中的蝴蝶群循环*/
        // model_matrix = glm::scale(model_matrix, glm::vec3(1.f, 1.f, 1.f) * scale_rand);
        // model_matrix = glm::rotate(model_matrix, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));                           // 正向
        // model_matrix = glm::translate(model_matrix, glm::vec3(translate_rand, translate_rand + 8.0f, translate_rand + 20.0f)); // 平移远一点
        // model_matrix = glm::rotate(model_matrix, glm::radians(rotate_rand), glm::vec3(0.0f, 1.0f, 0.0f));
        /*犹豫*/ /*高光*/
        // translate_rand = (float)((rand() % (200 - (-200))) + (-200)) / 100; // 模型位置偏移
        // model_matrix = glm::scale(model_matrix, glm::vec3(1.f, 1.f, 1.f) * scale_rand);
        // model_matrix = glm::rotate(model_matrix, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));                           // 正向
        // model_matrix = glm::translate(model_matrix, glm::vec3(translate_rand, translate_rand + 8.0f, translate_rand + 20.0f)); // 平移远一点
        // model_matrix = glm::rotate(model_matrix, glm::radians(rotate_rand), glm::vec3(0.0f, 1.0f, 0.0f));
    }
}

void GL_TASK::Butterfly::update_matrix()
{
    animator.UpdateAnimation(1 / 30.);
    float duration = danceAnimation.GetDuration();
    CurrentTime += danceAnimation.GetTicksPerSecond() * 1 / 30.;
    CurrentTime = fmod(CurrentTime, duration);
    // 使用正弦函数而非线性插值计算位置变换
    float t = CurrentTime / duration * 2; // 0.0~2.0f
    // t = sin(t * glm::pi<float>());
    glm::mat4 positiontranslate = glm::mat4(1.0f);
    if (mainButterfly)
        /*蝴蝶原地飞*/
        //;
        /*蝴蝶贴水飞*/
        positiontranslate = glm::translate(glm::mat4(1.0f), glm::vec3(1.5f * sin(t * 5 * glm::pi<float>()), 0.0f, -t * 0.05));
    /*犹豫*/
    //;
    /*高光*/
    // positiontranslate = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -t * 4));
    else
        positiontranslate = glm::translate(glm::mat4(1.0f), glm::vec3(sin(t * glm::pi<float>()) / 5, sin(t * glm::pi<float>()) / 3, 0.0f));

    printf("model_matrix: %s\n", glm::to_string(model_matrix).c_str());
    printf("positiontranslate: %s\n", glm::to_string(positiontranslate).c_str());
    model_matrix = positiontranslate * model_matrix;
    printf("model_matrix: %s\n", glm::to_string(model_matrix).c_str());

    keyframe_transforms_r = model_matrix * animator.m_KeyframeTransforms["R"];
    keyframe_transforms_l = model_matrix * animator.m_KeyframeTransforms["L"];
}