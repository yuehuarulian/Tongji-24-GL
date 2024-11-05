#ifndef _IMAGE_H_
#define _IMAGE_H_

#include <string>
#include <glm/glm.hpp>

namespace Basic
{
    typedef unsigned char uByte;
    class Image
    {
    public:
        // Pixel 用于表示图像中的一个像素
        template <typename T>
        struct Pixel
        {
            const size_t channel;
            T r; // 红red
            T g; // 绿green
            T b; // 蓝blue
            T a; // 透明度alpha
            Pixel(size_t channel) : channel(channel) {};
            Pixel(T r, T g, T b) : r(r), g(g), b(b), channel(3) {};
            Pixel(T r, T g, T b, T a) : r(r), g(g), b(b), a(a), channel(4) {};
            T &operator[](size_t idx);
            const T &operator[](size_t idx) const;
        };

        Image();
        Image(size_t width, size_t height, size_t channel);
        Image(const char *fileName, bool flip = false);
        ~Image();

        // 获取有关图像的信息
        bool IsValid() const { return data != NULL && type != ENUM_SRC_TYPE_INVALID; }
        uByte *GetData() { return !IsValid() ? NULL : data; }
        const uByte *GetConstData() const { return !IsValid() ? NULL : data; }
        size_t GetWidth() const { return width; }
        size_t GetHeight() const { return height; }
        size_t GetChannel() const { return channel; }

        // 为图像的某一个位置设置像素点
        bool SetPixel(size_t x, size_t y, const Pixel<uByte> &pixel);
        bool SetPixel(size_t x, size_t y, const Pixel<float> &pixel);
        bool SetPixel(size_t x, size_t y, const Pixel<double> &pixel);
        bool SetPixel(size_t x, size_t y, const glm::vec3 &pixel);

        // 获取图像某一个位置的像素点信息
        Pixel<uByte> GetPixel_UB(size_t x, size_t y) const;
        Pixel<float> GetPixel_F(size_t x, size_t y) const;
        Pixel<double> GetPixel_D(size_t x, size_t y) const;
        uByte &At(size_t x, size_t y, size_t channel);
        const uByte &At(size_t x, size_t y, size_t channel) const;

        bool Load(const std::string &fileName, bool flip = false);            // 加载stb图像信息
        void GenBuffer(size_t width, size_t height, size_t channel);          // 生成存放数据的缓冲区
        void Free();                                                          // 释放存放数据的缓冲区
        bool SaveAsPNG(const std::string &fileName, bool flip = false) const; // 将图像存储为PNG格式

        // 静态转换函数
        static Pixel<uByte> Pixel_F2UB(const Pixel<float> &pixel);
        static Pixel<uByte> Pixel_D2UB(const Pixel<double> &pixel);
        static Pixel<float> Pixel_UB2F(const Pixel<uByte> &pixel);
        static Pixel<double> Pixel_UB2D(const Pixel<uByte> &pixel);

        // 删除拷贝构造函数和赋值操作符
        Image &operator=(const Image &img) = delete;

    private:
        enum ENUM_SRC_TYPE
        {
            ENUM_SRC_TYPE_INVALID,
            ENUM_SRC_TYPE_NEW,
            ENUM_SRC_TYPE_STB,
        };
        uByte *data;
        size_t width;
        size_t height;
        size_t channel;
        ENUM_SRC_TYPE type;
    };
}

#endif