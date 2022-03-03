#version 450

layout (location = 0) in vec3 fragColor;
layout (location = 1) in vec3 fragPosWorld;
layout (location = 2) in vec3 fragNormalWorld;
layout (location = 3) in vec2 uv;
layout (location = 4) in vec4 inShadowCoord;

layout (location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform GlobalUbo {
  mat4 projection;
  mat4 view;
  
  mat4 lightMVP;
  vec3 directionalLightDirection; 
  vec4 ambientLightColor; // w is intensity
  
  vec3 lightPosition;
  vec4 lightColor; // w is intensity
} global_ubo;

layout (set = 0, binding = 1) uniform sampler2D shadowMap;

layout(set = 1, binding = 0) uniform sampler2D texSampler;

layout (push_constant) uniform Push {
    mat4 modelMatrix;
    mat4 normalMatrix;
} push;

const int enablePCF = 0;

float textureProj(vec4 shadowCoord, vec2 off)
{
	float shadow = 1.0;
	if ( shadowCoord.z > -1.0 && shadowCoord.z < 1.0 ) 
	{
		float dist = texture( shadowMap, shadowCoord.st + off ).r;
		if ( shadowCoord.w > 0.0 && dist < shadowCoord.z ) 
		{
			shadow = 0.1;
		}
	}
	return shadow;
}

float filterPCF(vec4 sc)
{
	ivec2 texDim = textureSize(shadowMap, 0);
	float scale = 1.5;
	float dx = scale * 1.0 / float(texDim.x);
	float dy = scale * 1.0 / float(texDim.y);

	float shadowFactor = 0.0;
	int count = 0;
	int range = 1;
	
	for (int x = -range; x <= range; x++)
	{
		for (int y = -range; y <= range; y++)
		{
			shadowFactor += textureProj(sc, vec2(dx*x, dy*y));
			count++;
		}
	
	}
	return shadowFactor / count;
}

void main(){

    float shadow = (enablePCF == 1) ? filterPCF(inShadowCoord / inShadowCoord.w) : textureProj(inShadowCoord / inShadowCoord.w, vec2(0.0));

    vec3 directionToLight = global_ubo.lightPosition - fragPosWorld;
    float attenuation = 1.0 / dot(directionToLight, directionToLight); // distance squared

    // ambiant
    vec3 ambientLight = global_ubo.ambientLightColor.xyz * global_ubo.ambientLightColor.w;
    
    // point light
    // vec3 lightColor = global_ubo.lightColor.xyz * global_ubo.lightColor.w * attenuation;
    // vec3 diffuseLight = lightColor * max(dot(normalize(fragNormalWorld), normalize(directionToLight)), 0);

    vec3 directionalLightColor = {1.0, 1.0, 1.0};
    vec3 normalWorldSpace = normalize(mat3(push.normalMatrix)*fragNormalWorld);
    vec3 diffuseLight = directionalLightColor * max(dot(normalWorldSpace, -global_ubo.directionalLightDirection), 0);

    vec2 uv_reversed = vec2( uv.x, 1.0- uv.y);
    vec4 color = texture(texSampler, uv_reversed)*vec4((diffuseLight + ambientLight), 1.0);
    vec4 ambiantColor = texture(texSampler, uv_reversed)*vec4(ambientLight, 1.0);

    outColor = vec4(color.xyz * shadow + ambiantColor.xyz*(1-shadow), 1.0);
}