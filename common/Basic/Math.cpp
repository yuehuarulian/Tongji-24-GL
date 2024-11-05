#include <Basic/Math.hpp>
#include <random>

using namespace Basic;
using namespace glm;
using namespace std;

float Math::Gray(const vec3 &color)
{
    // 计算灰度值
    return color.r * 0.299f + color.g * 0.587f + color.b * 0.114f;
}

bool Math::ReFract(const vec3 &viewDir, const vec3 &normal, float ratioNiNt, vec3 &refractDir)
{
    // 计算光线的折射方向
    const vec3 ud = normalize(viewDir);                                  // 将实现方向单位化
    float cosTheta = dot(ud, normal);                                    // 计算入射光与法线的余弦值
    float discriminent = 1 - pow(ratioNiNt, 2) * (1 - pow(cosTheta, 2)); // 计算判别式，用于判断是否有折射

    if (discriminent < 0) // 不存在折射光线
        return false;

    refractDir = ratioNiNt * (ud - normal * cosTheta) - normal * sqrt(discriminent); // 计算折射光线的方向
    return true;
}

float Math::FresnelSchlick(const vec3 &viewDir, const vec3 &halfway, float ratioNtNi)
{
    float cosTheta = dot(normalize(viewDir), normalize(halfway)); // 计算视线方向和半程向量之间的余弦值
    float R0 = pow((ratioNtNi - 1) / (ratioNtNi + 1), 2);         // 计算基础反射率 R0
    float R = R0 + (1 - R0) * pow(1 - cosTheta, 5);               // 使用 Schlick 近似公式计算反射率
    return R;                                                     // 返回反射率 R
}

vec2 Math::Sphere2UV(const vec3 &normal)
{
    vec2 uv;
    // 计算球面上的点的经度(phi)和纬度(theta)
    float phi = atan2(normal.z, normal.x);
    float theta = asin(normal.y);
    // 将经度和纬度转换为UV坐标
    uv[0] = 1 - (phi + PI) / (2 * PI); // 将phi映射到 [0, 1]
    uv[1] = (theta + PI / 2) / PI;     // 将theta映射到 [0, 1]
    return uv;
}

vec4 Math::Intersect_RayTri(const vec3 &e, const vec3 &d, const vec3 &a, const vec3 &b, const vec3 &c)
{
    // 构建一个 3x3 的矩阵，表示三角形的边和射线的方向
    mat3 equation_A(vec3(a - b), vec3(a - c), d);

    // 如果矩阵的行列式接近零，则射线与三角形平行，返回无交点
    if (abs(determinant(equation_A)) < EPSILON)
        return vec4(0, 0, 0, 0);

    // 计算方程右边的向量
    vec3 equation_b = a - e;
    // 求解线性方程，得到三个系数 alpha, beta 和 gamma
    vec3 equation_X = inverse(equation_A) * equation_b;
    // 计算 alpha, beta 和 gamma
    float alpha = 1 - equation_X[0] - equation_X[1];
    // 返回 alpha 和方程解（包括 alpha, beta, gamma）
    return vec4(alpha, equation_X);
}

template <typename T>
T Math::Mean(const std::vector<T> &data)
{
    if (data.size() == 0)
        return static_cast<T>(0);

    T sum = static_cast<T>(0);
    for (size_t i = 0; i < data.size(); i++)
        sum += data[i];

    return sum / data.size();
}

template <typename T>
T Math::Variance(const std::vector<T> &data)
{
    if (data.size() <= 1)
        return static_cast<T>(0);

    T mean = Mean(data);
    T sum = static_cast<T>(0);
    for (size_t i = 0; i < data.size(); i++)
        sum += pow(data[i] - mean, 2);

    return sum / (data.size() - 1);
}

template <typename T>
T Math::min(const std::vector<T> &val)
{
    if (val.empty())
        return static_cast<T>(0);

    T rst = val[0];
    for (size_t i = 1; i < val.size(); i++)
        rst = glm::min(rst, val[i]);

    return rst;
}

template <typename T>
T Math::max(const std::vector<T> &val)
{
    if (val.empty())
        return static_cast<T>(0);

    T rst = val[0];
    for (size_t i = 1; i < val.size(); i++)
        rst = glm::max(rst, val[i]);

    return rst;
}
