#ifndef _MATERIAL_H_
#define _MATERIAL_H_

#include <RTX/MatVisitor.hpp>
#include <Basic/HeapObj.hpp>
#include <glm/glm.hpp>
#include <vector>

#define MATERIAL_SETUP(CLASS)                             \
    HEAP_OBJ_SETUP(CLASS)                                 \
public:                                                   \
    virtual void Accept(MatVisitor::Ptr matVisitor) const \
    {                                                     \
        matVisitor->Visit(CThis());                       \
    }

namespace RTX
{
    class Ray;

    // 表示三维空间中的一个顶点，包含位置、法向量和纹理坐标
    struct Vertex
    {
        Vertex(glm::vec3 pos = glm::vec3(0), glm::vec3 normal = glm::vec3(0, 0, 1), float u = 0, float v = 0);
        glm::vec3 pos;
        glm::vec3 normal;
        float u;
        float v;

        void Transform(const glm::mat4 &transform);
        void Transform(const glm::mat4 &transform, const glm::mat3 &normalTransform);
        // 用于计算三角形内任意一点的插值（插值算法基于α-β-γ重心坐标）
        static const Vertex Interpolate_Tri(const glm::vec3 &abg, const Vertex &A, const Vertex &B, const Vertex &C);
    };

    // 用于记录光线与物体交点的信息，存储射线、交点位置、法向量和纹理坐标等信息。
    struct HitRecord
    {
        HitRecord(Basic::Ptr<Ray> ray = NULL, const glm::vec3 &pos = glm::vec3(0),
                  const glm::vec3 &normal = glm::vec3(0, 0, 1), float u = 0, float v = 0);

        Basic::Ptr<Ray> ray;
        Vertex vertex;
    };

    // 抽象基类:用于定义各种材质的基础行为。
    class Material : public Basic::HeapObj
    {
        HEAP_OBJ_SETUP(Material)
    public:
        // 返回值为 true 说明光线继续传播
        // 返回值为 false 说明光线不再传播
        virtual bool Scatter(const HitRecord &rec) const = 0;      // 纯虚函数:决定光线与材质交互后的行为
        virtual void Accept(MatVisitor::Ptr matVisitor) const = 0; // 用于接受 MatVisitor 访问器，从而根据材质类型执行访问操作，实现了访问者模式。
    };
}

#endif