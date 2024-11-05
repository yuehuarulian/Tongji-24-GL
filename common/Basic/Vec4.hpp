#ifndef _Vec4_H_
#define _Vec4_H_

namespace Basic
{
    template <typename T>
    class Vec4
    {
    public:
        Vec4() = default;
        Vec4(const T &x, const T &y, const T &z, const T &w)
        {
            data[0] = x;
            data[1] = y;
            data[2] = z;
            data[3] = w;
        }
        T &X() { return data[0]; }
        T &Y() { return data[1]; }
        T &Z() { return data[2]; }
        T &W() { return data[3]; }
        T &R() { return data[0]; }
        T &G() { return data[1]; }
        T &B() { return data[2]; }
        T &A() { return data[3]; }
        T *GetData() { return &(data[0]); }
        Vec4 &operator+(const Vec4 &v);
        Vec4 &operator-(const Vec4 &v);
        Vec4 &operator*(float a);
        Vec4 &operator/(float a);
        Vec4 &operator=(const Vec4 &v);

    private:
        T data[4];
    };

    template <typename T>
    Vec4<T> &Vec4<T>::operator+(const Vec4<T> &v)
    {
        data[0] += v.data[0];
        data[1] += v.data[1];
        data[2] += v.data[2];
        data[3] += v.data[3];
        return *this;
    }

    template <typename T>
    Vec4<T> &Vec4<T>::operator-(const Vec4<T> &v)
    {
        data[0] -= v.data[0];
        data[1] -= v.data[1];
        data[2] -= v.data[2];
        data[3] -= v.data[3];
        return *this;
    }

    template <typename T>
    Vec4<T> &Vec4<T>::operator*(float a)
    {
        data[0] *= a;
        data[1] *= a;
        data[2] *= a;
        data[4] *= a;
        return *this;
    }

    template <typename T>
    Vec4<T> &Vec4<T>::operator/(float a)
    {
        data[0] /= a;
        data[1] /= a;
        data[2] /= a;
        data[3] /= a;
        return *this;
    }

    template <typename T>
    Vec4<T> &Vec4<T>::operator=(const Vec4<T> &v)
    {
        data[0] = v.data[0];
        data[1] = v.data[1];
        data[2] = v.data[2];
        data[3] = v.data[3];
        return *this;
    }
}

#endif // !_Vec4_H_