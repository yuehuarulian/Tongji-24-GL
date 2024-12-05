#include "fluid.hpp"
#include "glm/gtx/transform.hpp"

GL_TASK::Fluid::Fluid(std::vector<Mesh *> &meshes, std::vector<MeshInstance *> &meshInstances,
                    std::vector<Texture *> &textures, std::vector<Material> &materials)
    : RenderableModel(meshes, meshInstances, textures, materials) {
    fluid_sim.pause(); // 初始先暂停
}

// 添加模型
bool GL_TASK::Fluid::add_model(const std::string &modelfilePath)
{
    Model *model = new Model();
    if (model->LoadFromFile(modelfilePath))
    {
        // 1. 将model中的纹理数据导入scene中
        int textureStartId = textures.size();
        for (auto texture : model->getTextures())
            textures.push_back(texture);

        // 2. 将model中的网格数据导入scene中
        for (auto mesh : model->getMeshes())
        {
            int mesh_id = meshes.size();
            int materialStartId = materials.size();

            // 3. 将mesh中的材质信息导入scene中
            //    同时更新纹理索引
            mesh->material.updateTexId(textureStartId);
            materials.push_back(mesh->material);
            // 4. 根据网格id和材质id创建一个meshInstance
            MeshInstance *instance = new MeshInstance(mesh_id, materialStartId, model_matrix);

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
    // 绑定mesh和更新信号
    fluid_sim.BindMesh(meshes[meshes.size() - 1]);
    return true;
}

void GL_TASK::Fluid::set_model_matrix(const glm::mat4 &model)
{
    model_matrix = glm::scale(model, glm::vec3(1.f, 1.f, 1.f) * (1.f / float(fluid_sim.get_scale()))); // Adjust scale
}

glm::mat4 GL_TASK::Fluid::get_model_matrix() const
{
    return model_matrix;
}

// 模拟控制函数
void GL_TASK::Fluid::start()
{
    fluid_sim.start();
}
void GL_TASK::Fluid::pause()
{
    fluid_sim.pause();
}
void GL_TASK::Fluid::advance()
{
    fluid_sim.advance();
}
void GL_TASK::Fluid::wait_until_next_frame(int frame)
{
    fluid_sim.wait_until_next_frame(frame);
}

#include <fstream>   // 文件流库
#include <nlohmann/json.hpp> // 配置文件库
// 自动计算水位高度
double GL_TASK::Fluid::get_water_level(const glm::vec3& roomMin, const glm::vec3& roomMax) const {
    // 加载配置文件
    nlohmann::json config;
    std::ifstream config_file("fluid_config.json");
    config_file >> config;
    // 从配置文件读取参数
    double _le = config.value("water_level", 0.35);
    // 计算水面高度
    return _le * (roomMax.y - roomMin.y) + roomMin.y;
}