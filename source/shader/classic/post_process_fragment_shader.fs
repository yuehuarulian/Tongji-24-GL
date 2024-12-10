#version 330

out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D pathTraceTexture;
uniform float invSampleCounter; //  1/N

void main()
{
    vec4 col = texture(pathTraceTexture, TexCoords) * invSampleCounter;
    vec3 color = col.xyz;
    float alpha = col.w;
    
    const float gamma = 2.2;
    float exposure = 1.0; 
    color = vec3(1.0) - exp(-color * exposure);
    color = pow(color, vec3(1.0 / gamma)); 

    FragColor = vec4(color, 1.0);
}