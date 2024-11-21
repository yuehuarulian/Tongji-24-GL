# version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;

out vec2 TexCoords;
out vec3 Normal_worldspace;
out vec3 viewDirection_worldspace;
out vec3 Pos_worldspace;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 camPos;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);

    vec3 vertexPosition_worldspace = (model * vec4(aPos, 1)).xyz;

    Normal_worldspace = (transpose(inverse(model)) * vec4(aNormal, 0)).xyz;

    viewDirection_worldspace = normalize(camPos - vertexPosition_worldspace);

    TexCoords = aTexCoords;
    Pos_worldspace = aPos;
}