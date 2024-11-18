/*// 与pbr.vs相同
# version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;

out vec2 TexCoords;
out vec3 WorldPos;
out vec3 Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    TexCoords = aTexCoords;
    WorldPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;

    gl_Position = projection * view * vec4(WorldPos, 1.0);
}*/
#version 330 core

// 输入属性
layout(location = 0) in vec3 aPos;    // 顶点位置
layout(location = 1) in vec3 aNormal; // 顶点法线
layout(location = 2) in vec2 aTexCoords;

// Uniforms
uniform mat4 keyframeTransforms; // 假设最多支持 100 个关键帧（调整根据需求）
uniform int activeKeyframeIndex;      // 当前活动关键帧索引

// 其他常用 Uniform
uniform mat4 model;       // 模型矩阵
uniform mat4 view;        // 视图矩阵
uniform mat4 projection;  // 投影矩阵

// 输出到片段着色器的变量
out vec2 TexCoords;
out vec3 WorldPos;  // 世界空间的片段位置
out vec3 Normal;   // 世界空间的法线

void main() {
    // 获取当前活动关键帧的变换矩阵
    //mat4 currentTransform = keyframeTransforms[activeKeyframeIndex];
    mat4 currentTransform = keyframeTransforms;

    // 应用变换到顶点位置
    vec4 transformedPosition = currentTransform * vec4(aPos, 1.0);

    // 应用变换到法线（只取变换矩阵的旋转/缩放部分，即 3x3 矩阵）
    mat3 normalMatrix = mat3(currentTransform);
    vec3 transformedNormal = normalize(normalMatrix * aNormal);

    // 输出到片段着色器的变量
    TexCoords = aTexCoords;
    WorldPos = vec3(model * transformedPosition);
    Normal = normalize(mat3(transpose(inverse(model))) * transformedNormal);

    // 最终顶点位置输出到屏幕空间
    gl_Position = projection * view * vec4(WorldPos, 1.0);
}
