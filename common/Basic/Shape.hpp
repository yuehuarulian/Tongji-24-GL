#ifndef _SHAPE_H_
#define _SHAPE_H_

#include <Basic/Array2D.hpp>

namespace Basic
{
    class Shape
    {
    public:
        Shape(size_t vertexNum, size_t triNum = 0);
        virtual ~Shape();
        float *GetVertexArr() { return (vertexArr == NULL) ? NULL : vertexArr->GetData(); }
        size_t GetTriNum() { return triNum; }
        size_t GetVertexArrSize() { return vertexArr->GetMemSize(); }

    protected:
        static const float PI;
        Array2D<float> *vertexArr;
        size_t triNum;
        size_t vertexNum;
    };
}

#endif