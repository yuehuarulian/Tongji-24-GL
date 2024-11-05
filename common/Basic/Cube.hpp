#ifndef _CUBE_H_
#define _CUBE_H_

#include <Basic/Shape.hpp>

namespace Basic
{
    class Cube : public Shape
    {
    public:
        Cube();
        virtual ~Cube();
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
        static const float cubeData[192];
    };
}

#endif