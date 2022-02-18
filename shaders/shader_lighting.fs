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
uniform SpotLight spotLights[2];

in vec3 FragPos;  
in vec3 Normal;  
in vec2 TexCoords;
in mat3 TBN;
in vec4 diffuse_input;
in vec4 specular_input;
in vec4 FragPosLightSpace;

uniform float useDiffuseMap;
uniform float coin;
uniform float useNormalMap;
uniform float useSpecularMap;
uniform float useShadow;
uniform float useLighting;
uniform sampler2D depthMapSampler;
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, float shadow);

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
    for(int x = -2; x <= 2; ++x)
    {
        for(int y = -2; y <= 2; ++y)
        {
            float pcfDepth = texture(depthMapSampler, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
        }    
    }
    shadow /= 25.0;
    return shadow;
}

void main()
{
    vec4 color = texture(material.diffuseSampler, TexCoords);
    if (useDiffuseMap<0.5){
        color = diffuse_input;
    }
	if (useLighting < 0.5f){
        FragColor = color;
        return; 
    }
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    float shadow = ShadowCalculation(FragPosLightSpace);
    vec3 result;
    for(int i = 0; i < spotLights.length(); i++)
        result += CalcSpotLight(spotLights[i], norm, FragPos, viewDir,shadow);    
    float brightness = dot(result, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 1.0)
        BrightColor = vec4(result, 1.0);
    else
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);    
    FragColor = vec4(result, 1);
}

// calculates the color when using a directional light.
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // combine results
    vec3 ambient = 0.05* vec3(texture(material.diffuseSampler, TexCoords).rgb);
    vec3 diffuse = light.color * diff * vec3(texture(material.diffuseSampler, TexCoords));
    
    if (useDiffuseMap<0.5f){
        ambient = 0.05*diffuse_input.rgb;
        diffuse = light.color * diff * diffuse_input.rgb;
    }
    vec3 specular = light.color * spec * vec3(texture(material.specularSampler, TexCoords));
    if (useSpecularMap<0.5f){
        specular =  light.color * spec * specular_input.rgb;
    }
    //return ambient;
    return ambient+diffuse+specular;
}

// calculates the color when using a spot light.
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, float shadow)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    // spotlight intensity
    float theta = dot(lightDir, normalize(-light.direction)); 
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    // combine results
    vec3 ambient = light.ambient * vec3(texture(material.diffuseSampler, TexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuseSampler, TexCoords));
    if (useDiffuseMap<0.5f){
        ambient = 0.05*diffuse_input.rgb;
        diffuse = light.diffuse * diff * diffuse_input.rgb;
    }
    if (coin>0.5f){
        ambient = 0.7*diffuse_input.rgb;
    }
    
    vec3 specular = light.specular * spec * vec3(texture(material.specularSampler, TexCoords));
    if (useSpecularMap<0.5f){
        specular =  light.specular * spec * specular_input.rgb;
    }
    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;
    return (ambient + (1-shadow)*diffuse + specular);
}