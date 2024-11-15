#version 330 core

out vec4 FragColor;

in vec3 FragPos; // 来自顶点着色器的世界坐标
in vec3 Normal;  // 来自顶点着色器的法线

uniform vec3 camPos;           // 摄像机位置
uniform vec3 liquidColor = vec3(0.1, 0.6, 0.8); // 水的浅蓝色 (RGB)
uniform float transparency = 0.7;               // 水的透明度 (0 ~ 1)
uniform float refractionIndex = 1.33;           // 水的折射率 (默认为 1.33)

uniform samplerCube skybox; // 用于环境反射的天空盒

void main()
{
    // 归一化法线
    vec3 N = normalize(Normal);
    
    // 视线方向
    vec3 V = normalize(camPos - FragPos);

    // 折射计算
    vec3 refracted = refract(-V, N, 1.0 / refractionIndex);

    // 从天空盒获取环境折射颜色
    vec3 refractedColor = texture(skybox, refracted).rgb;

    // 混合环境颜色与液体颜色
    vec3 color = mix(liquidColor, refractedColor, 0.5);

    // 最终颜色添加透明度
    FragColor = vec4(color, transparency);
}
