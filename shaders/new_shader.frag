#version 450

layout (location = 0) in vec3 fragColor;
layout (location = 1) in vec3 fragPosWorld;
layout (location = 2) in vec3 fragNormalWorld;
layout (location = 3) in vec2 uv;

layout (location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform GlobalUbo {
  mat4 projection;
  mat4 view;

  vec3 directionalLightDirection; 
  vec4 ambientLightColor; // w is intensity
  
  vec3 lightPosition;
  vec4 lightColor; // w is intensity
} global_ubo;


layout(set = 1, binding = 0) uniform sampler2D texSampler;

layout (push_constant) uniform Push {
    mat4 modelMatrix;
    mat4 normalMatrix;
} push;

void main(){

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
    outColor = texture(texSampler, uv_reversed)*vec4((diffuseLight + ambientLight), 1.0);
}