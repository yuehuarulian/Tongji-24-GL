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
    float exposure = 0.1; 
    // color = color / (color + vec3(1.0)); // Reinhard tone mapping
    color = vec3(1.0) - exp(-color * exposure);
    color = pow(color, vec3(1.0 / 2.2));

    FragColor = vec4(color, 1.0);
}