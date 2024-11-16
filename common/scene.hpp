#ifndef SCENE_HPP
#define SCENE_HPP
#include <vector>
#include <memory>
#include "glad/glad.h"
#include "room.hpp"
#include "shader_manager.hpp"
#include "renderable_model.hpp"
#include "light_manager.hpp"
#include "BVHConverter.hpp"
#include "Quad.hpp"

struct Indices
{
    unsigned int x, y, z;
};

class Scene
{
public:
    Scene(ShaderManager &shader_manager, LightManager light_manager) : shader_manager(shader_manager), light_manager(light_manager)
    {
        sceneBVH = new BVH(10.0f, 64, false);
        quad = new Quad();
    }

    virtual ~Scene() = default;

    virtual void render(const glm::mat4 &projection, const glm::mat4 &view, glm::vec3 &camera_pos) = 0;

protected:
    virtual void setup_scene() = 0;

    // 存储所有的网格
    std::vector<Mesh *> meshes;
    std::vector<MeshInstance *> meshInstances;

    ShaderManager &shader_manager; // 着色器管理者
    LightManager light_manager;    // 灯光管理者
    std::vector<std::shared_ptr<RenderableModel>> models;

    // Quad
    Quad *quad;

    // Mesh Data
    std::vector<Indices> vertIndices;
    std::vector<glm::vec4> verticesUVX; // Vertex + texture Coord (u/s)
    std::vector<glm::vec4> normalsUVY;  // Normal + texture Coord (v/t)
    std::vector<glm::mat4> transforms;

    // BVH
    BVHConverter bvhConverter; // 将网格数据转换为flatten
    BVH *sceneBVH;
    AABB sceneBounds;

    // 传递给GPU的数据
    GLuint BVHBuffer; // Flattened BVH Node Data
    GLuint BVHTex;
    GLuint vertexIndicesBuffer;
    GLuint vertexIndicesTex;
    GLuint verticesBuffer;
    GLuint verticesTex;
    GLuint normalsBuffer;
    GLuint normalsTex;
    GLuint transformsTex;

    bool AddModel(const std::string &modelfilePath, glm::mat4 transformMat = glm::mat4(1.0f));
    bool AddTexture(const std::string &filename);

protected:
    void createBLAS(); // 建立低层次的BVH加速结构
    void createTLAS(); // 建立高层次的BVH加速结构
    void copyMeshData();
    void InitGPUData();
    virtual void InitShaders() = 0;
};

#endif // SCENE_HPP