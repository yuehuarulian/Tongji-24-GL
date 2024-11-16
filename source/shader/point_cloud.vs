#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 viewPos; // 相机位置，用于计算距离

out vec3 FragPos; // 将片段位置传递到片段着色器

void main()
{
    // 计算世界空间中的位置
    vec4 worldPosition = model * vec4(aPos, 1.0);
    FragPos = worldPosition.xyz;

    // 设置 gl_Position
    gl_Position = projection * view * worldPosition;

    // 根据相机距离动态调整点大小
    float distance = length(viewPos - FragPos);
    float pointSize = 100.0 / distance; // 距离越远，点越小
    gl_PointSize = clamp(pointSize, 5.0, 20.0); // 限制点大小的范围
}
