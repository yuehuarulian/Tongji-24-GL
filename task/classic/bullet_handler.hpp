#include <btBulletDynamicsCommon.h>
#include <memory>
#include <iostream>
#include "fluid.hpp"

// btRigidBody *createBoat(btDiscreteDynamicsWorld *dynamicsWorld, GL_TASK::Boat *boatInstance)
// {
//     // 使用复杂的船形状，比如凸包
//     btCollisionShape *boatShape = new btConvexHullShape();
//     static_cast<btConvexHullShape *>(boatShape)->addPoint(btVector3(-0.5f, -0.5f, -0.5f));
//     static_cast<btConvexHullShape *>(boatShape)->addPoint(btVector3(0.5f, -0.5f, -0.5f));
//     static_cast<btConvexHullShape *>(boatShape)->addPoint(btVector3(0.5f, -0.5f, 0.5f));
//     static_cast<btConvexHullShape *>(boatShape)->addPoint(btVector3(-0.5f, -0.5f, 0.5f));
//     static_cast<btConvexHullShape *>(boatShape)->addPoint(btVector3(0, 0.5f, 0));
//     btDefaultMotionState *boatMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 5, 0)));
//     btScalar mass = 10.0f;
//     btVector3 boatInertia(0, 0, 0);
//     boatShape->calculateLocalInertia(mass, boatInertia);
//     btRigidBody::btRigidBodyConstructionInfo boatRigidBodyCI(mass, boatMotionState, boatShape, boatInertia);
//     btRigidBody *boatRigidBody = new btRigidBody(boatRigidBodyCI);
//     dynamicsWorld->addRigidBody(boatRigidBody);
//     // 设置船的刚体
//     boatInstance->setRigidBody(boatRigidBody);
//     return boatRigidBody;
// }

btDiscreteDynamicsWorld *initBullet(GL_TASK::Boat *boatInstance, const glm::vec3 &room_min, const glm::vec3 &room_max, const glm::vec3 &boat_start_position)
{
    // Bullet initialization
    btBroadphaseInterface *broadphase = new btDbvtBroadphase();
    btDefaultCollisionConfiguration *collisionConfiguration = new btDefaultCollisionConfiguration();
    btCollisionDispatcher *dispatcher = new btCollisionDispatcher(collisionConfiguration);
    btSequentialImpulseConstraintSolver *solver = new btSequentialImpulseConstraintSolver;

    btDiscreteDynamicsWorld *dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration);
    dynamicsWorld->setGravity(btVector3(0, -9.81f, 0)); // 设置重力方向为向下

    // **移除地面创建的代码部分** - 这里不需要地面

    // Create boat
    btCollisionShape *boatShape = new btBoxShape(btVector3(8.0f, 8.0f, 8.0f));

    // 确保船的初始位置在房间内
    glm::vec3 boatStartPosition = glm::clamp(boat_start_position, room_min, room_max); // 将初始位置限制在房间范围内

    // 创建船体的运动状态和刚体
    btDefaultMotionState *boatMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(boatStartPosition.x, boatStartPosition.y, boatStartPosition.z)));
    btScalar mass = 1.0f;
    btVector3 boatInertia(0, 0, 0);
    boatShape->calculateLocalInertia(mass, boatInertia);
    btRigidBody::btRigidBodyConstructionInfo boatRigidBodyCI(mass, boatMotionState, boatShape, boatInertia);
    btRigidBody *boatRigidBody = new btRigidBody(boatRigidBodyCI);

    // 将船体添加到物理世界
    dynamicsWorld->addRigidBody(boatRigidBody);

    // Set the boat instance rigid body
    boatInstance->setRigidBody(boatRigidBody);

    return dynamicsWorld;
}

void updatePhysics(btDiscreteDynamicsWorld *dynamicsWorld, float deltaTime)
{
    if (dynamicsWorld)
    {
        dynamicsWorld->stepSimulation(deltaTime, 10);
    }
}

void cleanupBullet(btDiscreteDynamicsWorld *dynamicsWorld, btBroadphaseInterface *broadphase,
                   btCollisionDispatcher *dispatcher, btSequentialImpulseConstraintSolver *solver,
                   btCollisionConfiguration *collisionConfiguration)
{
    if (dynamicsWorld)
    {
        // 删除刚体对象
        for (int i = dynamicsWorld->getNumCollisionObjects() - 1; i >= 0; --i)
        {
            btCollisionObject *obj = dynamicsWorld->getCollisionObjectArray()[i];
            dynamicsWorld->removeCollisionObject(obj);
            delete obj;
        }

        // 删除动态世界
        delete dynamicsWorld;
    }

    // 删除 Bullet 相关的其他组件
    delete solver;
    delete dispatcher;
    delete collisionConfiguration;
    delete broadphase;
}

// 在适当的位置定义转换函数
btVector3 to_btVector3(const fluid::vec3d &vec)
{
    return btVector3(static_cast<float>(vec.x), static_cast<float>(vec.y), static_cast<float>(vec.z));
}
btVector3 to_btVector3(const fluid::vec3d& vec, double scale) {
    return btVector3(static_cast<float>(vec.x / scale), static_cast<float>(vec.y / scale), static_cast<float>(vec.z / scale));
}
btVector3 transformPosition(const fluid::vec3d& position, const glm::mat4& matrix) {
    glm::vec4 glmPosition(position.x, position.y, position.z, 1.0);
    glm::vec4 transformed = matrix * glmPosition;
    return btVector3(transformed.x, transformed.y, transformed.z);
}
btVector3 transformVelocity(const fluid::vec3d& velocity, const glm::mat4& matrix) {
    glm::vec4 glmVelocity(velocity.x, velocity.y, velocity.z, 0.0); // 0.0 for velocity
    glm::vec4 transformed = matrix * glmVelocity;
    return btVector3(transformed.x, transformed.y, transformed.z);
}

void applyFluidForces(std::shared_ptr<GL_TASK::Boat> boat, std::shared_ptr<GL_TASK::Fluid> fluid)
{
    if (!boat || !fluid)
        return;

    // 获取船的世界变换
    btTransform boatTransform;
    boat->getRigidBody()->getMotionState()->getWorldTransform(boatTransform);
    btVector3 boatCenter = boatTransform.getOrigin();
    printf("boatCenter: %f %f %f\n", boatCenter.getX(), boatCenter.getY(), boatCenter.getZ());
    // 获取船测世界坐标
    glm::mat4 boat_model_matrix = boat->get_calculated_model_matrix();
    btVector3 boatPos = btVector3(boat_model_matrix[3][0], boat_model_matrix[3][1], boat_model_matrix[3][2]);
    printf("boatPos: %f %f %f\n", boatPos.getX(), boatPos.getY(), boatPos.getZ());
    btVector3 boatHalfExtents = static_cast<btBoxShape*>(boat->getRigidBody()->getCollisionShape())->getHalfExtentsWithMargin();
    //printf("boatHalfExtents: %f %f %f\n", boatHalfExtents.getX(), boatHalfExtents.getY(), boatHalfExtents.getZ());
    // 获取船体的 AABB 范围
    btVector3 minBounds = boatPos - boatHalfExtents;
    btVector3 maxBounds = boatPos + boatHalfExtents;

    // 获取流体粒子
    double p_scale = fluid->fluid_sim.get_scale();
    glm::mat4 fluid_model_matrix = fluid->get_model_matrix();
    std::vector<fluid::simulation::particle> particles = fluid->fluid_sim.get_particles();

    // 初始化浮力
    btVector3 totalForce(0, 0, 0);
    btVector3 totalTorque(0, 0, 0);

    for (const auto &particle : particles)
    {
        // 获取粒子位置和速度
        btVector3 particlePos = transformPosition(particle.position, fluid_model_matrix);
        if (particlePos.getX() < minBounds.getX() || particlePos.getX() > maxBounds.getX() ||
            particlePos.getY() < minBounds.getY() || particlePos.getY() > maxBounds.getY() ||
            particlePos.getZ() < minBounds.getZ() || particlePos.getZ() > maxBounds.getZ()) { // 排除无关粒子
            continue;
        }
        btVector3 particleVel = transformVelocity(particle.velocity, fluid_model_matrix);
        // 计算粒子压力
        //float fluid_density = 1000.0f;             // 水的密度 (kg/m^3)
        //float gravity = 9.8f;                      // 重力加速度 (m/s^2)
        //float depth = -70.f - particlePos.getX(); // 计算深度
        //float pressure = std::max(fluid_density * gravity * depth, 0.0f);
        float pressure = particle.pressure / 1000.0 / p_scale * p_scale * p_scale / 100.0;

        // 计算浮力，划分区域
        float submergedDepth = std::max(0.0f, float(particlePos.getY() - minBounds.getY())); //minBounds.getY()
        btVector3 buoyancyForce(0, pressure * submergedDepth, 0);
        if (submergedDepth > 0) {
            //printf("postion: %f vs %f\n", particlePos.getY(), boatPos.getY());
            //printf("pressure: %f submergedDepth: %f\n", pressure, submergedDepth);
            //printf("buoyancyForce: %f %f %f\n", buoyancyForce.getX(), buoyancyForce.getY(), buoyancyForce.getZ());
        }

        // 计算阻力 (根据相对速度)
        btVector3 relativeVelocity = boat->getRigidBody()->getLinearVelocity() - particleVel;
        // btVector3 dragForce = -fluid->dragCoefficient * relativeVelocity.length() * relativeVelocity;
        btVector3 dragForce = -0.04 * relativeVelocity.length() * relativeVelocity;
        // printf("dragForce: %f %f %f\n", dragForce.getX(), dragForce.getY(), dragForce.getZ());

        // 累积力
        totalForce += buoyancyForce + dragForce;

        // 计算扭矩 (以粒子到质心的向量计算)
        btVector3 relativePosition = particlePos - boatPos;
        totalTorque += relativePosition.cross(buoyancyForce + dragForce);
    }

    // 应用力和扭矩到船体
    boat->getRigidBody()->applyCentralForce(totalForce);
    boat->getRigidBody()->applyTorque(totalTorque);
    printf("totalForce: %f %f %f\n", totalForce.getX(), totalForce.getY(), totalForce.getZ());
    printf("totalTorque: %f %f %f\n", totalTorque.getX(), totalTorque.getY(), totalTorque.getZ());
}

void cleanup_physics(btDiscreteDynamicsWorld *dynamicsWorld)
{
    delete dynamicsWorld;
    dynamicsWorld = nullptr;
}

float calculateRenderTime()
{
    static auto lastTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> deltaTime = currentTime - lastTime;
    lastTime = currentTime;
    return deltaTime.count();
}
