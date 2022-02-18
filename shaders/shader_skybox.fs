#version 330 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

in vec3 TexCoords;

uniform samplerCube skyboxTexture1;

void main()
{
    FragColor = vec4(texture(skyboxTexture1, TexCoords).rgb, 1.0f);
    BrightColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);
}
