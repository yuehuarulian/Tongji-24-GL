#include "bullet_world.hpp"
#include "fluid.hpp"

namespace GL_TASK {

BulletWorld::BulletWorld(std::vector<Mesh *> &meshes, std::vector<MeshInstance *> &meshInstances, std::vector<Texture *> &textures, std::vector<Material> &materials)
    : roomMin(-400, -400, -400), roomMax(100, 100, 100), RenderableModel(meshes, meshInstances, textures, materials) {
    init();
}

BulletWorld::~BulletWorld() {
    stop();
    cleanup();
}

void BulletWorld::init() {
    broadphase = new btDbvtBroadphase();
    collisionConfiguration = new btDefaultCollisionConfiguration();
    dispatcher = new btCollisionDispatcher(collisionConfiguration);
    solver = new btSequentialImpulseConstraintSolver;

    dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration);
    dynamicsWorld->setGravity(btVector3(0, -9.81f, 0)); // 默认重力
}

void BulletWorld::cleanup() {
    for (btRigidBody* body : objects) {
        dynamicsWorld->removeRigidBody(body);
        delete body->getMotionState();
        delete body;
    }
    delete dynamicsWorld;
    delete solver;
    delete dispatcher;
    delete collisionConfiguration;
    delete broadphase;
}

void BulletWorld::setRoomBounds(const glm::vec3& room_min, const glm::vec3& room_max) {
    roomMin = to_btVector3(room_min);
    roomMax = to_btVector3(room_max);
}

void BulletWorld::BindFluid(std::shared_ptr<Fluid> fluidPtr) {
    fluid = fluidPtr;
    sim_time = fluid->fluid_sim.get_time();
    sim_dt = fluid->fluid_sim.get_time_step();
}

bool BulletWorld::bind_model(const std::string &modelfilePath, const ObjectType &bodyType) {
    // 删除当前绑定的模型信息
    if (model) {
        delete model;
        model = nullptr;  // 避免悬空指针
    }
    // 开始绑定新模型
    model = new Model();
    if (!model) {
        std::cout << "ERROR::UNABLE TO ALLOCATE MEMORY FOR NEW MODEL" << std::endl;
        return false;
    }
    if (!model->LoadFromFile(modelfilePath)) {
        std::cout << "ERROR::UNABLE TO LOAD MESH::" << modelfilePath << std::endl;
        delete model;
        model = nullptr;
        return false;
    }
    rigidBodyInfo.chooseType(bodyType);
    modelBaseMatrix = rigidBodyInfo.getBaseMatrix();
    std::cout << "BulletWorld::BIND a new " << rigidBodyInfo.getTypeName() << " model." << std::endl;
    return true;
}
bool BulletWorld::add_model() {
    float x = rand() % int(roomMax.getX() - roomMin.getX()) + roomMin.getX();
    float y = rand() % int(roomMax.getY() - roomMin.getY()) + roomMin.getY();
    float z = rand() % int(roomMax.getZ() - roomMin.getZ()) + roomMin.getZ();
    glm::vec3 world_position = glm::vec3(x, y, z);
    return add_model(world_position);
}
bool BulletWorld::add_model(const glm::vec3 &world_position) {
    glm::mat4 world_matrix = glm::translate(glm::mat4(1.0f), world_position);
    return add_model(world_matrix);
}
bool BulletWorld::add_model(const glm::mat4 &world_matrix) {
    if (model == nullptr) {
        std::cout << "ERROR::MODEL IS NOT LOADED" << std::endl;
        return false;
    }
    std::cout << "BulletWorld::ADD a new " << rigidBodyInfo.getTypeName() << " model." << std::endl;
    // 1. 将model中的纹理数据导入scene中
    int textureStartId = textures.size();
    for (auto texture : model->getTextures())
        textures.push_back(texture);

    // 2. 将model中的网格数据导入scene中
    std::vector<glm::mat4*> objMats;
    for (auto mesh : model->getMeshes())
    {
        int mesh_id = meshes.size();
        int materialStartId = materials.size();

        // 3. 将mesh中的材质信息导入scene中
        //    同时更新纹理索引
        mesh->material.updateTexId(textureStartId);
        materials.push_back(mesh->material);
        // 4. 根据网格id和材质id创建一个meshInstance
        MeshInstance *instance = new MeshInstance(mesh_id, materialStartId, world_matrix * modelBaseMatrix);

        meshes.push_back(mesh);
        meshInstances.push_back(instance);
        // 6.添加变换矩阵
        objMats.push_back(&(instance->transform));
    }

    // 5. 添加刚体，绑定模型信息
    addObject(glmToBtTransform(world_matrix), rigidBodyInfo);

    // 6.添加变换矩阵
    objectBaseMatrices.push_back(modelBaseMatrix);
    objectMatrices.push_back(objMats);
    return true;
}
void BulletWorld::addObject(const btTransform& state, const RigidBodyManager& info) {
    // 获取刚体所需的数据
    btScalar mass = info.getMass();
    btCollisionShape* shape = info.getCollisionShape();
    btScalar friction = info.getFriction();
    btScalar restitution = info.getRestitution();
    btDefaultMotionState* motionState = new btDefaultMotionState(state);
    // 计算惯性
    btVector3 inertia(0, 0, 0);
    if (mass != 0.0f)
        shape->calculateLocalInertia(mass, inertia);  // 只有质量不为零时才需要计算惯性

    btRigidBody::btRigidBodyConstructionInfo rigidBodyCI(mass, motionState, shape, inertia);
    btRigidBody* rigidBody = new btRigidBody(rigidBodyCI);
    rigidBody->setFriction(friction);
    rigidBody->setRestitution(restitution);
    // 添加到物理世界
    std::lock_guard<std::mutex> lock(worldMutex);
    dynamicsWorld->addRigidBody(rigidBody);
    objects.push_back(rigidBody);
}

void BulletWorld::start() {
    if (!fluid) {
        printf("BulletWorld:: fluid is not binded! Failed to start physics simulation!");
        return;
    }
    running = true;
    physicsThread = std::thread(&BulletWorld::updateLoop, this);
}

void BulletWorld::stop() {
    if (running) {
        running = false;
        if (physicsThread.joinable()) {
            physicsThread.join();
        }
    }
}

void BulletWorld::updateLoop() {
    while (running) {
        std::cout << "BulletWorld:: start to update next frame." << std::endl;
        {
            std::lock_guard<std::mutex> lock(worldMutex);
            std::cout << "BulletWorld:: Applying Fluid Forces." << std::endl;
            applyFluidForces();
            std::cout << "BulletWorld:: Simulating next step: " << sim_dt << "s" << std::endl;
            dynamicsWorld->stepSimulation(sim_dt, 5);
            enforceBounds();
            std::cout << "BulletWorld:: Applying model Matrices." << std::endl;
            applyModelMatrices();
        }
        sim_time += sim_dt;
        if (dirty) *dirty = true;
        std::cout << "BulletWorld:: finish a new frame." << std::endl;
        fluid->fluid_sim.wait_until_next_sim();
    }
}

void BulletWorld::enforceBounds() {
    for (btRigidBody* body : objects) {
        btTransform transform;
        body->getMotionState()->getWorldTransform(transform);
        btVector3 position = transform.getOrigin();

        // Clamp position to the room bounds
        position = btClamp(position, roomMin, roomMax);

        transform.setOrigin(position);
        body->setWorldTransform(transform);
        body->getMotionState()->setWorldTransform(transform);
    }
}

// 判断点是否在刚体的碰撞体内部
bool isPointInsideRigidBody(const btRigidBody* rigidBody, const btVector3& Pos)
{
    if (!rigidBody)
        return false; // 如果刚体为空，直接返回

    // 获取刚体的碰撞形状
    const btCollisionShape* shape = rigidBody->getCollisionShape();
    if (!shape)
        return false; // 如果没有碰撞形状，返回 false

    // 获取刚体的变换矩阵
    btTransform worldTransform = rigidBody->getWorldTransform();
    
    // 将点转换到刚体的局部坐标系
    btVector3 localPos = worldTransform.invXform(Pos);

    // 判断不同碰撞形状类型
    switch (shape->getShapeType())
    {
        case BOX_SHAPE_PROXYTYPE:
        {
            // 对于盒子形状（btBoxShape）
            const btBoxShape* boxShape = static_cast<const btBoxShape*>(shape);
            btVector3 halfExtents = boxShape->getHalfExtentsWithMargin();
            // 检查点是否在盒子范围内
            return (localPos.getX() >= -halfExtents.getX() && localPos.getX() <= halfExtents.getX() &&
                    localPos.getY() >= -halfExtents.getY() && localPos.getY() <= halfExtents.getY() &&
                    localPos.getZ() >= -halfExtents.getZ() && localPos.getZ() <= halfExtents.getZ());
        }

        case SPHERE_SHAPE_PROXYTYPE:
        {
            // 对于球形状（btSphereShape）
            const btSphereShape* sphereShape = static_cast<const btSphereShape*>(shape);
            float radius = sphereShape->getRadius();
            // 检查点是否在球的范围内
            return localPos.length() <= radius;
        }

        case CYLINDER_SHAPE_PROXYTYPE:
        {
            // 对于圆柱形状（btCylinderShape）
            const btCylinderShape* cylinderShape = static_cast<const btCylinderShape*>(shape);
            btVector3 halfExtents = cylinderShape->getHalfExtentsWithMargin();
            // 检查点是否在圆柱的范围内
            float radius = halfExtents.getX();
            float height = halfExtents.getY();
            return (localPos.length2() <= radius * radius && localPos.getZ() >= -height && localPos.getZ() <= height);
        }

        case CONE_SHAPE_PROXYTYPE:
        {
            // 对于圆锥形状（btConeShape）
            const btConeShape* coneShape = static_cast<const btConeShape*>(shape);
            float radius = coneShape->getRadius();
            float height = coneShape->getHeight();
            // 检查点是否在圆锥的范围内
            float z = localPos.getZ();
            float maxRadius = radius * (1 - z / height);  // 圆锥半径随高度变化
            return (localPos.length2() <= maxRadius * maxRadius && z >= 0 && z <= height);
        }

        case COMPOUND_SHAPE_PROXYTYPE:
        /*{
            // 对于复合形状（btCompoundShape），需要递归检查所有子形状
            const btCompoundShape* compoundShape = static_cast<const btCompoundShape*>(shape);
            for (int i = 0; i < compoundShape->getNumChildShapes(); ++i)
            {
                btCollisionShape* childShape = compoundShape->getChildShape(i);
                btTransform childTransform = compoundShape->getChildTransform(i);
                btVector3 localChildPos = childTransform.invXform(localPos);
                
                // 递归检查该子形状
                if (childShape->isInside(localChildPos)) {
                    return true;
                }
            }
            return false;
        }*/

        default:
            return false; // 如果是其他形状类型，返回 false
    }
}

// 计算浮力作用
void BulletWorld::applyFluidForces()
{
    if (!fluid)
        return;

    // 获取流体粒子
    double p_scale = fluid->fluid_sim.get_scale();
    glm::mat4 fluid_model_matrix = fluid->get_model_matrix();
    std::vector<fluid::simulation::particle> particles = fluid->fluid_sim.get_particles();

    // 初始化浮力和总扭矩
    std::vector<btVector3> totalForces(objects.size(), btVector3(0, 0, 0));
    std::vector<btVector3> totalTorques(objects.size(), btVector3(0, 0, 0));

    for (const auto &particle : particles)
    {
        // 获取粒子世界位置和速度
        btVector3 particlePos = transformPosition(particle.position, fluid_model_matrix);
        btVector3 particleVel = transformVelocity(particle.velocity, fluid_model_matrix);
        // 获取粒子压力
        float pressure = particle.pressure / p_scale * p_scale * p_scale / 100.0;

        // 遍历所有的刚体，计算浮力和扭矩
        for (size_t i = 0; i < objects.size(); ++i)
        {
            btRigidBody* rigidBody = objects[i];
            if (rigidBody && isPointInsideRigidBody(rigidBody, particlePos)) {
                btTransform bodyTransform;
                rigidBody->getMotionState()->getWorldTransform(bodyTransform);
                btVector3 bodyPos = bodyTransform.getOrigin();

                // 计算浮力，划分区域
                //float submergedDepth = std::max(0.0f, float(particlePos.getY() - minBounds.getY()));  // 浸没深度
                btVector3 buoyancyForce(0, pressure, 0);  // 浮力

                // 计算阻力 (根据相对速度)
                btVector3 relativeVelocity = rigidBody->getLinearVelocity() - particleVel;
                btVector3 dragForce = -0.04 * relativeVelocity.length() * relativeVelocity;  // 简化阻力计算

                // 累积力
                totalForces[i] += buoyancyForce + dragForce;

                // 计算扭矩
                btVector3 relativePosition = particlePos - bodyPos;
                totalTorques[i] += relativePosition.cross(buoyancyForce + dragForce);
            }
        }
    }

    // 遍历所有的刚体，并将计算的浮力和扭矩作用于刚体
    for (size_t i = 0; i < objects.size(); ++i)
    {
        btRigidBody* rigidBody = objects[i];
        if (rigidBody) {
            rigidBody->applyCentralForce(totalForces[i]); // 应用浮力            
            rigidBody->applyTorque(totalTorques[i]); // 应用扭矩
            printf("totalForce%d: %f %f %f\n", i, totalForces[i].getX(), totalForces[i].getY(), totalForces[i].getZ());
            printf("totalTorque%d: %f %f %f\n", i, totalTorques[i].getX(), totalTorques[i].getY(), totalTorques[i].getZ());
        }
    }
}

// 为每个模型更新变换矩阵
void BulletWorld::applyModelMatrices() {
    for (size_t i = 0; i < objects.size(); ++i) {
        if (objects[i] && i < objectMatrices[i].size()) {
            btTransform bodyTransform;
            objects[i]->getMotionState()->getWorldTransform(bodyTransform);
            glm::mat4 world_matrix = glm::mat4(1.0f);
            bodyTransform.getOpenGLMatrix(glm::value_ptr(world_matrix));
            printf("Rigid body #%d position: %f %f %f\n", i, world_matrix[3][0], world_matrix[3][1], world_matrix[3][2]);
            glm::mat4 model_matrix = objectBaseMatrices[i];
            
            for (auto pmat : objectMatrices[i])
                *pmat = world_matrix * model_matrix;
        }
    }
}

btDiscreteDynamicsWorld* BulletWorld::getDynamicsWorld() const {
    return dynamicsWorld;
}

// 内置转换函数
btTransform BulletWorld::glmToBtTransform(const glm::mat4& glmMatrix) const
{
    // 提取平移部分：矩阵的最后一列
    glm::vec3 position = glm::vec3(glmMatrix[3]);

    // 提取旋转部分：矩阵的左上 3x3 子矩阵
    glm::mat3 rotationMatrix = glm::mat3(glmMatrix);

    // 将旋转矩阵转换为 btMatrix3x3
    btMatrix3x3 btRotation(
        rotationMatrix[0][0], rotationMatrix[0][1], rotationMatrix[0][2],
        rotationMatrix[1][0], rotationMatrix[1][1], rotationMatrix[1][2],
        rotationMatrix[2][0], rotationMatrix[2][1], rotationMatrix[2][2]
    );

    // 创建并返回 btTransform 对象
    btTransform btTransform(btRotation, btVector3(position.x, position.y, position.z));

    return btTransform;
}
btVector3 BulletWorld::to_btVector3(const glm::vec3& glmVec)
{
    return btVector3(glmVec.x, glmVec.y, glmVec.z);
}
btVector3 BulletWorld::to_btVector3(const fluid::vec3d &vec)
{
    return btVector3(static_cast<float>(vec.x), static_cast<float>(vec.y), static_cast<float>(vec.z));
}
btVector3 BulletWorld::to_btVector3(const fluid::vec3d& vec, double scale) {
    return btVector3(static_cast<float>(vec.x / scale), static_cast<float>(vec.y / scale), static_cast<float>(vec.z / scale));
}
btVector3 BulletWorld::transformPosition(const fluid::vec3d& position, const glm::mat4& matrix) {
    glm::vec4 glmPosition(position.x, position.y, position.z, 1.0);
    glm::vec4 transformed = matrix * glmPosition;
    return btVector3(transformed.x, transformed.y, transformed.z);
}
btVector3 BulletWorld::transformVelocity(const fluid::vec3d& velocity, const glm::mat4& matrix) {
    glm::vec4 glmVelocity(velocity.x, velocity.y, velocity.z, 0.0); // 0.0 for velocity
    glm::vec4 transformed = matrix * glmVelocity;
    return btVector3(transformed.x, transformed.y, transformed.z);
}
// btClamp 函数
float BulletWorld::btClamp(float value, float min, float max) {
    return std::max(min, std::min(value, max));
}
btVector3 BulletWorld::btClamp(const btVector3& position, const btVector3& min, const btVector3& max) {
    float x = btClamp(position.getX(), min.getX(), max.getX());
    float y = btClamp(position.getY(), min.getY(), max.getY());
    float z = btClamp(position.getZ(), min.getZ(), max.getZ());
    return btVector3(x, y, z);
}

} // namespace GL_TASK
