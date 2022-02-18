#version 330 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

struct Material {
    sampler2D diffuseSampler;
    sampler2D specularSampler;
    sampler2D normalSampler;
    float shininess;
}; 

struct DirLight {
    vec3 direction;
    //vec3 ambient;
    //vec3 diffuse;
    //vec3 specular;// this is I_d (I_s = I_d, I_a = 0.3 * I_d)
    vec3 color;
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

int NR_SPOT_LIGHTS = 1;
uniform vec3 viewPos;
uniform Material material;
uniform DirLight dirLight;
uniform SpotLight spotLights[1];

in vec3 FragPos;  
in vec3 Normal;  
in vec2 TexCoords;
in mat3 TBN;
//in vec4 FragPosLightSpace;

uniform float useNormalMap;
uniform float useSpecularMap;
uniform float useShadow;
uniform float useLighting;
uniform sampler2D depthMapSampler;

float ShadowCalculation(vec4 fragPosLightSpace)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;     // perform perspective divide
    projCoords = projCoords * 0.5 + 0.5;     // transform to [0,1] range
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(depthMapSampler, projCoords.xy).r; 
    float currentDepth = projCoords.z; // get depth of current fragment from light's perspective   
    float bias = 0.001; // check whether current frag pos is in shadow
    //float shadow = (currentDepth-bias) > closestDepth ? 1.0 : 0.0;
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(depthMapSampler, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(depthMapSampler, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0;
    return shadow;
}

void main()
{
	vec4 color = texture(material.diffuseSampler, TexCoords);
    FragColor = color;
    BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
}
