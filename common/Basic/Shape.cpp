#include <Basic/Shape.hpp>

using namespace Basic;

const float Shape::PI = 3.1415926;

Shape::Shape(size_t vertexNum, size_t triNum)
    : triNum(triNum), vertexNum(vertexNum)
{
    vertexArr = new Array2D<float>(vertexNum, 3);
}

Shape::~Shape()
{
    delete vertexArr;
    vertexArr = NULL;
}
