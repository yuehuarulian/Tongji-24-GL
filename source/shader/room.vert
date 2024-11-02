# version 330 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoords;

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
    gl_Position = P * V * M * vec4(position, 1.0);

    vec3 vertexPosition_worldspace = (M * vec4(position, 1)).xyz;

    Normal_worldspace = (transpose(inverse(M)) * vec4(normal, 0)).xyz;

    viewDirection_worldspace = normalize(CameraPosition_worldspace - vertexPosition_worldspace);

    TexCoords = texCoords;
    Pos_worldspace = position;
}