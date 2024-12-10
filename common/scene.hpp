#ifndef SCENE_HPP
#define SCENE_HPP
#include <vector>
#include <memory>
#include "glad/glad.h"
#include "shader_manager.hpp"
#include "renderable_model.hpp"
#include "light_manager.hpp"
#include "BVHConverter.hpp"
#include "Quad.hpp"
#include "camera_control.hpp"

struct Indices
{
    unsigned int x, y, z;
};

class Scene
{
public:
    Scene(ShaderManager &shader_manager, LightManager &light_manager, const int WINDOW_WIDTH = 1080, const int WINDOW_HEIGHT = 720)
        : shader_manager(shader_manager), light_manager(light_manager), WINDOW_WIDTH(WINDOW_WIDTH), WINDOW_HEIGHT(WINDOW_HEIGHT)
    {
        currentBuffer = 0;
        dirty = false;
        sceneBVH = new BVH(10.0f, 64, false);
        quad = new Quad();
    }

    virtual ~Scene() = default;

    virtual void render(Camera &) = 0;
    virtual void present() = 0; // 展示渲染结果
    virtual void update() = 0;  // 进行数据更新
    virtual void wait_until_next_frame(int frame_number) = 0;

    void setDirty(bool isDirty) { this->dirty = isDirty; }
    bool getDirty() const { return this->dirty; }
    int getFrameNum() const { return this->frameNum; }

    void SaveFrame(const std::string filename);
    void DenoiseProcess();

protected:
    virtual void setup_scene() = 0;
    bool add_model(const std::string &modelfilePath, glm::mat4 transformMat = glm::mat4(1.0f));
    int add_material(const Material &material);
    int add_texture(const std::string &filename);

    void createBLAS(); // 建立低层次的BVH加速结构
    void createTLAS(); // 建立高层次的BVH加速结构
    void process_data();
    void init_GPU_data();
    void init_FBOs();
    void update_GPU_data();
    void update_FBOs();

    const int WINDOW_WIDTH, WINDOW_HEIGHT;
    const int texArrayHeight = 2048;
    const int texArrayWidth = 2048;
    int frameNum = 0;
    bool dirty; // 脏位

    // 存储所有的网格
    std::vector<Mesh *> meshes;
    std::vector<MeshInstance *> meshInstances;

    ShaderManager &shader_manager; // 着色器管理者
    LightManager &light_manager;   // 灯光管理者

    // 帧缓冲对象
    GLuint pathTraceFBO;
    GLuint accumFBO;
    GLuint outputFBO;
    // 帧缓冲对应的纹理
    GLuint pathTraceTexture;
    GLuint accumTexture;
    GLuint outputTexture[2];
    GLuint denoisedTexture;
    int currentBuffer; // 表示当前渲染结果存储的位置

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

    // 材质数据 Material
    std::vector<Material> materials;

    // 纹理数据 Texture
    std::vector<Texture *> textures;
    std::vector<unsigned char> textureMapsArray;

    // 传递给GPU的数据
    // ---------- BVH树节点数据 ---------- //
    GLuint BVHBuffer;
    GLuint BVHTex;
    // ---------- 顶点索引数据 ---------- //
    GLuint vertexIndicesBuffer;
    GLuint vertexIndicesTex;
    // ---------- 顶点/UV数据 ---------- //
    GLuint verticesBuffer;
    GLuint verticesTex;
    // ---------- 法线/UV数据 ---------- //
    GLuint normalsBuffer;
    GLuint normalsTex;
    // ---------- 转换矩阵数据 ---------- //
    GLuint transformsBuffer;
    GLuint transformsTex;
    // ---------- 材质数据 ---------- //
    GLuint materialsBuffer;
    GLuint materialsTex;
    // ---------- 纹理数据 ---------- //
    GLuint textureMapsArrayTex;

    // Denoiser output
    glm::vec3 *denoiserInputFramePtr;
    glm::vec3 *frameOutputPtr;
    bool denoised;

    bool AddModel(const std::string &modelfilePath, glm::mat4 transformMat = glm::mat4(1.0f));
    int AddMaterial(const Material &material);
    int AddTexture(const std::string &filename);

protected:
    const int texArrayHeight = 2048;
    const int texArrayWidth = 2048;
    int frameNum = 0;

    void createBLAS(); // 建立低层次的BVH加速结构
    void createTLAS(); // 建立高层次的BVH加速结构
    void ProcessData();
    void InitGPUData();
    void InitFBOs();
    virtual void InitShaders() = 0;
};

#endif // SCENE_HPP