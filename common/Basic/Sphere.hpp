#ifndef _SPHERE_H_
#define _SPHERE_H_

#include <Basic/Shape.hpp>

namespace Basic
{
    class Sphere : public Shape
    {
    public:
        Sphere(size_t n);
        virtual ~Sphere();
        float *GetNormalArr() { return (normalArr == NULL) ? NULL : normalArr->GetData(); }
        float *GetTexCoordsArr() { return (texCoordsArr == NULL) ? NULL : texCoordsArr->GetData(); }
        size_t *GetIndexArr() { return (indexArr == NULL) ? NULL : indexArr->GetData(); }
        size_t GetNormalArrSize() { return normalArr->GetMemSize(); }
        size_t GetTexCoordsArrSize() { return texCoordsArr->GetMemSize(); }
        size_t GetIndexArrSize() { return indexArr->GetMemSize(); }

    protected:
        Array2D<float> *normalArr;
        Array2D<float> *texCoordsArr;
        Array2D<size_t> *indexArr;
    };
}

#endif