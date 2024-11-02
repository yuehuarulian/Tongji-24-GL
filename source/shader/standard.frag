# version 330 core

// 定义光源结构体

struct PointLight {
    vec3 position;
    float constant;
    float linear;
    float quadratic;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct DirectionalLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;
    float constant;
    float linear;
    float quadratic;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

in vec2 TexCoords;
in vec3 Normal;
in vec3 viewPos;
in vec3 FragPos;

out vec4 FragColor;

#define NUM_POINT_LIGHTS 4
#define NUM_DIRECTIONAL_LIGHTS 2
#define NUM_SPOT_LIGHTS 1

// uniform 变量
uniform PointLight pointLights[NUM_POINT_LIGHTS];
uniform DirectionalLight directionalLights[NUM_DIRECTIONAL_LIGHTS];
uniform SpotLight spotLights[NUM_SPOT_LIGHTS];

uniform sampler2D texture_diffuse1;

vec3 calculatePointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 calculateDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDir);
vec3 calculateSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main()
{
    vec3 result = vec3(0.0);
    vec3 normal = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    // 遍历所有点光源
    for (int i = 0; i < NUM_POINT_LIGHTS; ++i)
        result += calculatePointLight(pointLights[i], normal, FragPos, viewDir);

    // 遍历所有平行光源
    for (int i = 0; i < NUM_DIRECTIONAL_LIGHTS; ++i)
        result += calculateDirectionalLight(directionalLights[i], normal, viewDir);

    // 遍历所有聚光灯
    for (int i = 0; i < NUM_SPOT_LIGHTS; ++i)
        result += calculateSpotLight(spotLights[i], normal, FragPos, viewDir);

    FragColor = vec4(result, 1.0);
}

vec3 calculatePointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
    vec3 lightDir = normalize(light.position - fragPos);
    float distance = length(light.position - fragPos);

    // 衰减
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    // 光照计算
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff;

    vec3 result = (light.ambient + diffuse) * attenuation;
    return result;
}
