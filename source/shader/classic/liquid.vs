#version 330 core

layout(location = 0) in vec3 aPos; // 顶点位置
layout(location = 1) in vec3 aNormal; // 顶点法线

out vec3 FragPos; // 世界坐标位置
out vec3 Normal;   // 法线

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0)); // 转换为世界坐标
    Normal = mat3(transpose(inverse(model))) * aNormal; // 变换法线
    gl_Position = projection * view * vec4(FragPos, 1.0); // 转换为裁剪空间坐标
}
