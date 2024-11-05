#include <Basic/Image.hpp>

using namespace std;

int main(int argc, char **argv)
{
    Basic::Image img(400, 400, 4); // 创建一个400*400*4的图像对象
    for (size_t i = 0; i < 400; i++)
    {
        for (size_t j = 0; j < 400; j++)
        {
            if ((i + j) % 2 == 0)
                img.SetPixel(i, j, Basic::Image::Pixel<float>(0, 0, 0, 0.5));
            else
                img.SetPixel(i, j, Basic::Image::Pixel<float>(10.0 + i / 400.0, 10.0 + i / 400.0, 0, 0.1));
        }
    }
    img.SaveAsPNG("text.png");
    return 0;
}
