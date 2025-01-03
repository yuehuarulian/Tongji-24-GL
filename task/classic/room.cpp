#include "room.hpp"
#include "glm/gtx/transform.hpp"

GL_TASK::Room::Room(const std::string &model_path, std::vector<Mesh *> &meshes, std::vector<MeshInstance *> &meshInstances, std::vector<Texture *> &textures, std::vector<Material> &materials)
    : RenderableModel(meshes, meshInstances, textures, materials)
{

    set_model_matrix();
    add_model(model_path);
}

void GL_TASK::Room::update() {};

bool GL_TASK::Room::add_model(const std::string &model_path)
{
    Model *model = new Model();
    if (model->LoadFromFile(model_path))
    {
        // 初始化边界信息
        min_bound = glm::vec3(std::numeric_limits<float>::max());
        max_bound = glm::vec3(std::numeric_limits<float>::lowest());

        // 1. 将model中的纹理数据导入scene中
        int textureStartId = this->textures.size();
        for (auto texture : model->getTextures())
            this->textures.push_back(texture);

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
            MeshInstance *instance = new MeshInstance(mesh_id, materialStartId, model_matrix);

            this->meshes.push_back(mesh);
            this->meshInstances.push_back(instance);

            // 5. 更新边界信息
            for (const auto &vertex : mesh->vertices)
            {
                glm::vec3 pos = glm::vec3(model_matrix * glm::vec4(vertex.Position, 1.0f));
                min_bound = glm::min(min_bound, pos);
                max_bound = glm::max(max_bound, pos);
            }
        }
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

void GL_TASK::Room::set_model_matrix()
{
    model_matrix = glm::mat4(1.0f);
    model_matrix = glm::rotate(model_matrix, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    model_matrix = glm::scale(model_matrix, glm::vec3(1.f, 1.f, 1.f) * 1.3f);
}

glm::mat4 GL_TASK::Room::get_model_matrix() const
{
    return model_matrix;
}

void GL_TASK::Room::getBoundingBox(glm::vec3 &min, glm::vec3 &max)
{
    min = min_bound;
    max = max_bound;
}