# version 330 core
struct PointLight {
    vec3 position;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
}; 

in vec2 TexCoords;
in vec3 Normal_worldspace;
in vec3 viewDirection_worldspace;
in vec3 Pos_worldspace;

out vec4 FragColor;

uniform sampler2D texture_diffuse1;
uniform PointLight pointlight;

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    float distance    = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + 
                 light.quadratic * (distance * distance));
    vec3 ambient  = light.ambient  * vec3(vec3(1,1,1));// 环境光
    vec3 diffuse  = light.diffuse  * diff * vec3(1,1,1);// 漫反射
    vec3 specular = light.specular * spec * vec3(texture(texture_diffuse1, TexCoords));// 镜面反射
    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}

void main()
{
    vec3 color = CalcPointLight(pointlight, Normal_worldspace, Pos_worldspace, viewDirection_worldspace);
    
    FragColor = vec4(color, 1.0);
    // FragColor = vec4(1.0,1.0,1.0,1.0);
}