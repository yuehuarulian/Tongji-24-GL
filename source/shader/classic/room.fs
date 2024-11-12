// 与pbr_texture.fs相同
#version 330 core
out vec4 FragColor;
in vec2 TexCoords;
in vec3 WorldPos;
in vec3 Normal;

const float PI = 3.14159265359;
const int MAX_AREA_LIGHTS = 10;

uniform vec3 camPos;

// PBR 材质贴图
uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform sampler2D texture_normal1;
uniform sampler2D texture_height1;
uniform sampler2D texture_metallic1;
uniform sampler2D texture_roughness1;
uniform sampler2D texture_ao1;

// 光源参数
struct AreaLight {
    vec3 position;   // 网面光源中心位置
    vec3 normal;     // 光源朝向
    vec3 color;      // 光源颜色
    float width;     // 宽度
    float height;    // 高度
    int num_samples;  // 采样点数
};
uniform int num_area_lights; // 当前场景中网面光源的数量
uniform AreaLight area_lights[MAX_AREA_LIGHTS];


// 从法线贴图中获取法线向量
vec3 getNormalFromMap() {
    vec3 tangentNormal = texture(texture_normal1, TexCoords).xyz * 2.0 - 1.0;

    vec3 Q1  = dFdx(WorldPos);
    vec3 Q2  = dFdy(WorldPos);
    vec2 st1 = dFdx(TexCoords);
    vec2 st2 = dFdy(TexCoords);

    vec3 N   = normalize(Normal);
    vec3 T  = normalize(Q1 * st2.t - Q2 * st1.t);
    vec3 B  = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}

// 各向同性 GGX 函数
float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

// 几何函数
float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

// 几何双向阴影遮蔽
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

// 菲涅尔近似
vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

// 网面光源采样点生成
vec3 sampleAreaLight(AreaLight light, float sampleX, float sampleY) {
    vec3 right = normalize(cross(light.normal, vec3(0.0, 1.0, 0.0)));
    vec3 up = normalize(cross(right, light.normal));
    vec3 samplePos = light.position + (sampleX - 0.5) * light.width * right + (sampleY - 0.5) * light.height * up;
    return samplePos;
}

void main() {
    vec3 albedo     = pow(texture(texture_diffuse1, TexCoords).rgb, vec3(2.2));
    float metallic  = texture(texture_metallic1, TexCoords).r;
    float roughness = texture(texture_roughness1, TexCoords).r;
    float ao        = texture(texture_ao1, TexCoords).r;

    vec3 N = getNormalFromMap();
    vec3 V = normalize(camPos - WorldPos);

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    vec3 Lo = vec3(0.0);

    // 网面光源的计算
    for(int i = 0; i < num_area_lights; i++) {
        AreaLight light = area_lights[i];
        vec3 areaLightColor = vec3(0.0);

        for(int j = 0; j < light.num_samples; j++) {
            float sampleX = fract(sin(float(j) * 43758.5453) * 2.0);
            float sampleY = fract(sin(float(j) * 12345.6789) * 2.0);
            vec3 samplePos = sampleAreaLight(light, sampleX, sampleY);

            vec3 L = normalize(samplePos - WorldPos);
            vec3 H = normalize(V + L);
            float distance = length(samplePos - WorldPos);
            float attenuation = 1.0 / (distance * distance);
            vec3 radiance = light.color * attenuation;

            float NDF = DistributionGGX(N, H, roughness);
            float G   = GeometrySmith(N, V, L, roughness);
            vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);

            vec3 numerator    = NDF * G * F;
            float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
            vec3 specular = numerator / denominator;

            vec3 kS = F;
            vec3 kD = vec3(1.0) - kS;
            kD *= 1.0 - metallic;

            float NdotL = max(dot(N, L), 0.0);
            areaLightColor += (kD * albedo / PI + specular) * radiance * NdotL;
        }
        Lo += areaLightColor / float(light.num_samples);
    }
    vec3 ambient = vec3(0.03) * albedo * ao;
    vec3 color = ambient + Lo;

    // 曝光映射
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));

    FragColor = vec4(color, 1.0);
}
