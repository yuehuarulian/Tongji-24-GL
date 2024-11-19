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
    btCollisionShape *boatShape = new btBoxShape(btVector3(0.5f, 0.5f, 0.5f));

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

void applyFluidForces(btRigidBody *boatBody, std::shared_ptr<GL_TASK::Fluid> fluid)
{
    if (!boatBody || !fluid)
        return;

    // 获取船的世界变换
    btTransform trans;
    boatBody->getMotionState()->getWorldTransform(trans);
    btVector3 position = trans.getOrigin();

    // 获取流体粒子
    auto particles = fluid->fluid_sim.get_particles();

    btVector3 totalForce(0, 0, 0);
    btVector3 totalTorque(0, 0, 0);

    for (const auto &particle : particles)
    {
        // 计算粒子压力 (假设 fluid 提供 calculate_pressure 方法)
        // float pressure = fluid->fluid_sim.calculate_pressure(particle.position);
        float fluid_density = 1000.0f;             // 水的密度 (kg/m^3)
        float gravity = 9.8f;                      // 重力加速度 (m/s^2)
        float depth = -70.f - particle.position.y; // 计算深度
        float pressure = std::max(fluid_density * gravity * depth, 0.0f);

        // 计算浮力
        float submergedDepth = std::max(0.0f, float(position.getY() - particle.position.y));
        btVector3 buoyancyForce(0, pressure * submergedDepth, 0);

        // 将粒子速度转换为 btVector3
        btVector3 particleVelocity(particle.velocity.x, particle.velocity.y, particle.velocity.z);

        // 计算阻力 (根据相对速度)
        btVector3 relativeVelocity = boatBody->getLinearVelocity() - particleVelocity;
        // btVector3 dragForce = -fluid->dragCoefficient * relativeVelocity.length() * relativeVelocity;
        btVector3 dragForce = -0.4 * relativeVelocity.length() * relativeVelocity;

        // 累积力
        totalForce += buoyancyForce + dragForce;

        // 计算扭矩 (以粒子到质心的向量计算)
        btVector3 relativePosition = to_btVector3(particle.position) - position;

        // btVector3 relativePosition = particle.position - position;
        totalTorque += relativePosition.cross(buoyancyForce + dragForce);
    }

    // 应用力和扭矩到船体
    boatBody->applyCentralForce(totalForce);
    boatBody->applyTorque(totalTorque);
    printf("totalForce: %f %f %f\n", totalForce.getX(), totalForce.getY(), totalForce.getZ());
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
