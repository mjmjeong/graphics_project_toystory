#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 Bitangent;
layout (location = 5) in ivec4 vertexBoneIds;
layout (location = 6) in vec4 vertexBoneWeights;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;
out mat3 TBN;

const int MAX_BONES = 16;
uniform mat4 world;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;
uniform mat4 bones[MAX_BONES];
uniform float jump_height;
uniform float useNormalMap;
struct Material {
    sampler2D diffuseSampler;
    sampler2D specularSampler;
    sampler2D normalSampler;
    float shininess;
}; 

uniform Material material;
uniform sampler2D depthMapSampler;
uniform float max_depth;
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
    float gap = currentDepth - closestDepth;
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
    return gap;
}

void main()
{

	TexCoords = aTexCoord;
	
    mat4 boneTransform = mat4(1.0);
    if(vertexBoneWeights.x != 0.0)
    {
        boneTransform = bones[vertexBoneIds.x] * mat4(vertexBoneWeights.x);
        boneTransform += bones[vertexBoneIds.y] * mat4(vertexBoneWeights.y);
        boneTransform += bones[vertexBoneIds.z] * mat4(vertexBoneWeights.z);
        boneTransform += bones[vertexBoneIds.w] * mat4(vertexBoneWeights.w);
    }
    vec4 fragPosLightSpace = lightSpaceMatrix * world * vec4(vec3(0), 1.0f);
    float gap;
    gap = ShadowCalculation(fragPosLightSpace);
    mat4 translatey =  mat4(1.0, 0.0, 0.0, 0.0,  0.0, 1.0, 0.0, 0.0,  0.0, 0.0, 1.0, 0.0,  0.0, gap*max_depth+jump_height, 0.0, 1.0);
    mat4 world_new = translatey*world;
    //mat4 world_new = world;
    
    vec4 position_world = world_new * boneTransform * vec4(aPos, 1.0f);
    gl_Position = projection * view * position_world;
    Normal = mat3(transpose(inverse(world))) * aNormal;	         
    FragPos = vec3(world * vec4(aPos, 1.0));
}
