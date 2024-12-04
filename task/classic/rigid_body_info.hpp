#ifndef RIGID_BODY_INFO_HPP
#define RIGID_BODY_INFO_HPP

#include <btBulletDynamicsCommon.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>

namespace GL_TASK {

// 物体类型枚举
enum class ObjectType
{
    NONE,
    BOAT,
    FLOWER
};

// 基础刚体信息配置类
class RigidBodyInfo
{
public:
    // 物体的基本信息
    ObjectType objectType;             // 物体类型（如船只、花朵等）
    btCollisionShape* collisionShape;  // 碰撞形状（例如：盒子、球体等）
    btScalar mass;                     // 物体质量
    btVector3 dimensions;              // 物体尺寸（例如：盒子的半长宽高）
    glm::vec3 position;                // 初始位置
    btScalar friction;                 // 摩擦力系数
    btScalar restitution;              // 反弹系数

    // 构造函数
    RigidBodyInfo(ObjectType type)
        : objectType(type), collisionShape(nullptr) {
        chooseType(type);
    }

    void chooseType(ObjectType type) {
        switch(type) {
            case ObjectType::BOAT:
                initBoat();
                break;
            case ObjectType::FLOWER:
                initFlower();
                break;
            default:
                initDefault();
                break;
        }
    }

    // 初始化默认刚体信息
    void initDefault()
    {
        objectType = ObjectType::NONE;
        dimensions = btVector3(1.0f, 1.0f, 1.0f);  // 默认尺寸
        mass = 1.0f;                               // 默认质量
        if (collisionShape) { delete collisionShape; collisionShape = nullptr; }
        collisionShape = new btSphereShape(dimensions.length()); // 默认使用球体形状
        position = glm::vec3(0.0f, 0.0f, 0.0f);      // 初始位置
        friction = restitution = 0.5f;
    }

    // 初始化船只刚体信息
    void initBoat()
    {
        objectType = ObjectType::BOAT;
        dimensions = btVector3(10.0f, 6.0f, 10.0f);  // 船体尺寸
        mass = 100.0f;                               // 船的质量
        if (collisionShape) { delete collisionShape; collisionShape = nullptr; }
        collisionShape = new btBoxShape(dimensions); // 船体使用盒子形状
        position = glm::vec3(0.0f, 5.0f, 0.0f);      // 初始位置
        friction = 0.3f;                             // 船体摩擦力
        restitution = 0.1f;                          // 船体反弹系数
    }

    // 初始化花朵刚体信息
    void initFlower()
    {
        objectType = ObjectType::FLOWER;
        dimensions = btVector3(1.0f, 3.0f, 1.0f);  // 花朵尺寸（可以是椭圆）
        mass = 1.0f;                               // 花朵的质量
        if (collisionShape) { delete collisionShape; collisionShape = nullptr; }
        collisionShape = new btCylinderShape(dimensions); // 花朵使用圆柱形状
        position = glm::vec3(0.0f, 3.0f, 0.0f);    // 初始位置
        friction = 0.1f;                           // 花朵摩擦力
        restitution = 0.0f;                        // 花朵不反弹
    }

    // 返回刚体碰撞形状(新副本)
    btCollisionShape* getCollisionShape() const {
        switch(objectType) {
            case ObjectType::BOAT:
                return new btBoxShape(dimensions);
            case ObjectType::FLOWER:
                return new btCylinderShape(dimensions);
            default:
                return new btSphereShape(dimensions.length());
        }
        return nullptr;
    }

    // 返回刚体质量
    btScalar getMass() const { return mass; }

    // 返回刚体的初始位置
    glm::vec3 getPosition() const { return position; }

    // 返回摩擦系数
    btScalar getFriction() const { return friction; }

    // 返回反弹系数
    btScalar getRestitution() const { return restitution; }

    string typeName() const {
        switch(objectType) {
            case ObjectType::NONE:
                return "NONE";
            case ObjectType::BOAT:
                return "BOAT";
            case ObjectType::FLOWER:
                return "FLOWER";
        } 
        return "error";
    }
};

}

#endif // RIGID_BODY_INFO_HPP
