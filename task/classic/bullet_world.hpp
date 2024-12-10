#ifndef BULLET_WORLD_H
#define BULLET_WORLD_H

#include <btBulletDynamicsCommon.h>
#include "renderable_model.hpp"
#include "rigid_body_info.hpp"
#include "fluid.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <memory>
#include <vector>
#include <mutex>
#include <thread>
#include <functional>

namespace GL_TASK 
{
    class BulletWorld : public RenderableModel
    {
    public:
        BulletWorld(std::vector<Mesh *> &meshes, std::vector<MeshInstance *> &meshInstances, std::vector<Texture *> &textures, std::vector<Material> &materials);
        ~BulletWorld();

        // 绑定脏位
        void BindDirty(bool* dirtyPtr) {dirty = dirtyPtr;}

        // 设置房间大小
        void setRoomBounds(const glm::vec3& roomMin, const glm::vec3& roomMax);

        // 绑定流体对象
        void BindFluid(std::shared_ptr<Fluid> fluidPtr);

        // 添加模型
        bool bind_model(const std::string &modelfilePath, const ObjectType &bodyType);
        bool add_model();
        bool add_model(const glm::vec3 &world_position);
        bool add_model(const glm::mat4 &world_matrix);

        // 开始/停止物理模拟
        void start();
        void stop();

        // 获取 Bullet 世界
        btDiscreteDynamicsWorld* getDynamicsWorld() const;

        // override
        void update() override {};
    protected:
        void set_model_matrix() override {};
        bool add_model(const std::string &model_path) override { return false; };

    private:
        void init();
        void cleanup();
        void updateLoop();
        void enforceBounds();

        // 添加刚体对象
        void addObject(const btTransform& state, const RigidBodyManager& info);

        // 内置应力
        void applyFluidForces();
        void applyModelMatrices();

        // 类型转换函数
        btTransform glmToBtTransform(const glm::mat4& glmMatrix) const;
        btVector3 to_btVector3(const glm::vec3& glmVec);
        btVector3 to_btVector3(const fluid::vec3d &vec);
        btVector3 to_btVector3(const fluid::vec3d& vec, double scale);
        btVector3 transformPosition(const fluid::vec3d& position, const glm::mat4& matrix);
        btVector3 transformVelocity(const fluid::vec3d& velocity, const glm::mat4& matrix);
        float btClamp(float value, float min, float max);
        btVector3 btClamp(const btVector3& position, const btVector3& min, const btVector3& max);

        // 绑定脏位
        bool* dirty{nullptr};

        // 模拟参数
        double sim_time{0};
        double sim_dt{1/60.0};
        btVector3 roomMin;
        btVector3 roomMax;

        // 物理世界
        btDiscreteDynamicsWorld* dynamicsWorld{nullptr};
        btBroadphaseInterface *broadphase;
        btDefaultCollisionConfiguration *collisionConfiguration;
        btCollisionDispatcher *dispatcher;
        btSequentialImpulseConstraintSolver *solver;

        // 模型初始化
        Model *model{nullptr};
        RigidBodyManager rigidBodyInfo{ObjectType::NONE};
        glm::mat4 modelBaseMatrix{glm::mat4(1.0f)};

        // 流体/刚体与变换矩阵指针
        std::shared_ptr<Fluid> fluid;
        std::vector<btRigidBody*> objects;
        std::vector<glm::mat4> objectBaseMatrices;
        std::vector<std::vector<glm::mat4*>> objectMatrices;

        // 线程同步
        std::mutex worldMutex;
        std::thread physicsThread;
        bool running{false};
    };
} // namespace GL_TASK

#endif // BULLET_WORLD_H
