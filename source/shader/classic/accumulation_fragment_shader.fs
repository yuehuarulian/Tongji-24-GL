#version 330

out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D imgTex;

void main()
{
    FragColor = texture(imgTex, TexCoords);
}