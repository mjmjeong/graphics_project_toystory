#version 330 core
out vec4 FragColor;
    //struct Material {
    //    sampler2D diffuseSampler;
    //    sampler2D specularSampler;
    //    sampler2D normalSampler;
    //    float shininess;
    //}; 
    //uniform Material material;
in vec2 TexCoords;
uniform sampler2D diffuse1;
void main()
{    
    //FragColor = texture(diffuseSampler, TexCoords);
    FragColor = texture(diffuse1, TexCoords);
    
}