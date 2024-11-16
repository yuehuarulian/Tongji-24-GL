#version 330 core
out vec4 FragColor;

in vec3 FragPos;

uniform vec3 viewPos;
uniform vec3 fogColor;
uniform float fogDensity;

void main()
{
    // 基本光散射
    float distance = length(FragPos - viewPos);
    float alpha = exp(-pow(distance * fogDensity, 2.0));
    alpha = clamp(alpha, 0.01, 0.8);
    
    vec3 color = mix(fogColor, vec3(0.9, 0.9, 1.0), alpha);

    // 输出颜色
    FragColor = vec4(color, alpha);
}
