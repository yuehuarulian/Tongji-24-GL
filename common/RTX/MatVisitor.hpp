#ifndef _MAT_VISITOR_H_
#define _MAT_VISITOR_H_

#include <Basic/HeapObj.hpp>

/*
    MatVisitor 类是一个材质访问器（Visitor），用于遍历和处理不同类型的材质（Material）对象。
    该类应用了访问者模式（Visitor Pattern），这种设计模式用于在不修改对象结构的前提下，向对象结构添加新的操作。
*/

namespace RTX
{
    // 声明各种材质类
    class Material;
    class Lambertian;
    class Metal;
    class Dielectric;
    class Light;
    class OpMaterial;
    class Isotropic;

    class MatVisitor : public Basic::HeapObj
    {
        HEAP_OBJ_SETUP(MatVisitor)
    public:
        // 访问不同材质类型的虚函数接口，每个函数用于处理一种特定的材质类型。
        // 当访问这些材质对象时，将根据对象的具体类型调用对应的 Visit 方法。

        // 默认材质访问函数，适用于基础材质 Material 的处理
        virtual void Visit(Basic::CPtr<Material> material);

        // 漫反射材质访问函数，适用于 Lambertian 材质的处理
        virtual void Visit(Basic::CPtr<Lambertian> lambertian);

        // 金属材质访问函数，适用于 Metal 材质的处理
        virtual void Visit(Basic::CPtr<Metal> metal);

        // 透明折射材质访问函数，适用于 Dielectric 材质的处理
        virtual void Visit(Basic::CPtr<Dielectric> dielectric);

        // 发光材质访问函数，适用于 Light 材质的处理
        virtual void Visit(Basic::CPtr<Light> light);

        // 操作材质访问函数，适用于 OpMaterial 材质的处理
        virtual void Visit(Basic::CPtr<OpMaterial> opMaterial);

        // 各向同性材质访问函数，适用于 Isotropic 材质的处理
        virtual void Visit(Basic::CPtr<Isotropic> isotropic);
    };
}

#endif