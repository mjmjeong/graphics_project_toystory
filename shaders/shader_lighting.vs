#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec3 aTangent;

uniform Mat{
	vec4 aAmbient;
	vec4 aDiffuse;
	vec4 aSpecular;
};
out vec3 FragPos;
out vec4 FragPosLightSpace;
out vec3 Normal;
out vec2 TexCoords;
out mat3 TBN;
out vec4 diffuse_input;
out vec4 specular_input;

uniform mat4 world;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;

uniform float useNormalMap;
struct Material {
    sampler2D diffuseSampler;
    sampler2D specularSampler;
    sampler2D normalSampler;
    float shininess;
}; 

uniform Material material;

void main()
{
	TexCoords = aTexCoord;
	FragPos = vec3(world * vec4(aPos, 1.0));
	gl_Position = projection * view * world * vec4(aPos, 1.0f);
	FragPosLightSpace = lightSpaceMatrix * world * vec4(aPos, 1.0f);

	// on-off by key 1 (useNormalMap).
    // if model does not have a normal map, this should be always 0.
    // if useNormalMap is 0, we use a geometric normal as a surface normal.
    // if useNormalMap is 1, we use a geometric normal altered by normal map as a surface normal.
	if(useNormalMap > 0.5f) {
        mat3 normalMatrix = transpose(inverse(mat3(world)));
        vec3 T = normalize(normalMatrix * aTangent);
        vec3 N = normalize(normalMatrix * aNormal);
        T = normalize(T - dot(T, N) * N);
        vec3 B = cross(N, T);
        TBN = transpose(mat3(T, B, N));    
	}
    Normal = mat3(transpose(inverse(world))) * aNormal;	         
    diffuse_input = aDiffuse;
    specular_input = aSpecular;
}
