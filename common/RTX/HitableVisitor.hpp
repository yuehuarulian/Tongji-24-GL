#ifndef _HITABLE_VISITOR_H_
#define _HITABLE_VISITOR_H_

#include <Basic/HeapObj.hpp>

namespace RTX
{
    // 声明不同类型的可命中对象
    class Hitable;
    class Sphere;
    class Sky;
    class MoveSphere;
    class Triangle;
    class Transform;
    class Volume;
    class Group;
    class BVH_Node;
    class TriMesh;
    class Model;

    // HitableVisitor 类是一个访问器类，用于访问和处理不同类型的可命中对象。
    // 通过访问者模式（Visitor Pattern），可以在不修改这些类的情况下添加新的操作。
    class HitableVisitor : public Basic::HeapObj
    {
        HEAP_OBJ_SETUP(HitableVisitor)
    public:
        // 访问不同可命中对象类型的虚函数接口。每个 Visit 函数用于处理一种特定类型的对象。
        // 这些函数可以根据具体类型执行特定的逻辑，符合访问者模式的设计。

        // 默认的 Hitable 访问函数，用于基础 Hitable 对象的处理
        virtual void Visit(Basic::CPtr<Hitable> sphere);

        // 访问函数，用于处理 Sphere（球体）类型的可命中对象
        virtual void Visit(Basic::CPtr<Sphere> sphere);

        // 访问函数，用于处理 Sky（天空）类型的可命中对象
        virtual void Visit(Basic::CPtr<Sky> sky);

        // 访问函数，用于处理 MoveSphere（运动球）类型的可命中对象
        virtual void Visit(Basic::CPtr<MoveSphere> moveSphere);

        // 访问函数，用于处理 Triangle（三角形）类型的可命中对象
        virtual void Visit(Basic::CPtr<Triangle> triangle);

        // 访问函数，用于处理 Transform（变换）类型的可命中对象
        virtual void Visit(Basic::CPtr<Transform> transform);

        // 访问函数，用于处理 Volume（体积）类型的可命中对象
        virtual void Visit(Basic::CPtr<Volume> volume);

        // 访问函数，用于处理 Group（组）类型的可命中对象
        virtual void Visit(Basic::CPtr<Group> group);

        // 访问函数，用于处理 BVH_Node（包围盒层次结构节点）类型的可命中对象
        virtual void Visit(Basic::CPtr<BVH_Node> bvhNode);

        // 访问函数，用于处理 TriMesh（三角网格）类型的可命中对象
        virtual void Visit(Basic::CPtr<TriMesh> triMesh);

        // 访问函数，用于处理 Model（模型）类型的可命中对象
        virtual void Visit(Basic::CPtr<Model> model);
    };
}

#endif