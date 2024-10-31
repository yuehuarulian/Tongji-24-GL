# version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;

out vec2 TexCoords;

out vec3 Normal_worldspace;
out vec3 viewDirection_worldspace;
out vec3 Pos_worldspace;

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;
uniform vec3 CameraPosition_worldspace;

void main()
{
    gl_Position = P * V * M * vec4(aPos, 1.0);

    vec3 vertexPosition_worldspace = (M * vec4(aPos, 1)).xyz;

    // vec3 LightPosition_cameraspace = (V * vec4(LightPosition_worldspace, 1)).xyz;

    // LightDirection_cameraspace =  normalize(LightPosition_cameraspace - vertexPosition_cameraspace); // 光照方向应该指向光源

    Normal_worldspace = (transpose(inverse(M)) * vec4(aNormal, 0)).xyz;

    viewDirection_worldspace = normalize(CameraPosition_worldspace - vertexPosition_worldspace);

    TexCoords = aTexCoords;
    Pos_worldspace = aPos;
}