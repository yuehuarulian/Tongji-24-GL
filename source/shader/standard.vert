#version 330 core

// 输入属性
layout(location = 0) in vec3 position;  // 顶点位置
layout(location = 1) in vec3 normal;    // 顶点法线
layout(location = 2) in vec2 texCoords; // 纹理坐标

// 输出给片段着色器的变量
out vec2 TexCoords;
out vec3 Normal;
out vec3 viewPos;
out vec3 FragPos;

// Uniform 变量
uniform mat4 M;       // 模型矩阵
uniform mat4 V;        // 视图矩阵
uniform mat4 P;  // 投影矩阵

void main()
{
    // 将模型坐标变换到世界坐标
    FragPos = vec3(M * vec4(position, 1.0));

    // 法线变换，使用 mat3 处理并进行逆转置
    Normal = mat3(transpose(inverse(M))) * normal;

    // 观察空间坐标，用于计算观察方向
    viewPos = vec3(inverse(V) * vec4(0.0, 0.0, 0.0, 1.0)); // 观察者位置在观察空间为原点

    // 纹理坐标传递给片段着色器
    TexCoords = texCoords;

    // 顶点坐标变换到裁剪空间
    gl_Position = P * V * vec4(FragPos, 1.0);
}
