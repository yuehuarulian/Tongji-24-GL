#ifndef _ARRAY_D_H_
#define _ARRAY_D_H_

#include <cstdlib>

namespace Basic
{
    template <typename T>
    class Array2D
    {
    public:
        Array2D(size_t row, size_t col) : row(row), col(col)
        {
            data = new T[row * col];
        }
        ~Array2D()
        {
            delete[] data;
            data = nullptr;
        }

        T &At(size_t row, size_t col) { return data[row * this->col + col]; }
        T &operator()(size_t row, size_t col) { return data[row * this->col + col]; }
        void Copy(size_t row, size_t col, size_t n, const T *src);
        size_t GetRow() const { return row; }
        size_t GetCol() const { return col; }
        size_t GetArrSize() const { return row * col; }
        size_t GetMemSize() const { return row * col * sizeof(T); }
        T *GetData() { return data; }

    private:
        const size_t row;
        const size_t col;
        T *data;
    };

    template <typename T>
    void Array2D<T>::Copy(size_t row, size_t col, size_t n, const T *src)
    {
        for (size_t i = 0; i < n; i++)
        {
            At(row, col++) = src[i];
            if (col == this->col)
            {
                row++;
                col = 0;
            }
        }
    }

}

#endif