#version 450

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 color;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec2 uv;

layout (location = 0) out vec3 fragColor;
layout (location = 1) out vec3 fragPosWorld;
layout (location = 2) out vec3 fragNormalWorld;
layout (location = 3) out vec2 uv_out;

layout (location = 4) out vec4 outShadowCoord;

layout(set = 0, binding = 0) uniform GlobalUbo {
  mat4 projection;
  mat4 view;

  mat4 lightMVP;
  vec3 directionalLightDirection; 
  vec4 ambientLightColor; // w is intensity
  
  vec3 lightPosition;
  vec4 lightColor; // w is intensity
} global_ubo;

// layout(set = 1, binding = 0) uniform MaterialUbo {
//     vec3 color;
// } object_ubo;

layout (push_constant) uniform Push {
    mat4 modelMatrix;
    mat4 normalMatrix;
} push;


const mat4 biasMat = mat4( 
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.5, 0.5, 0.0, 1.0 );

void main() {
    vec4 positionWorld = push.modelMatrix * vec4(inPos, 1.0);
    gl_Position = global_ubo.projection * global_ubo.view * positionWorld;

    fragNormalWorld = normalize(mat3(push.normalMatrix)*normal);
    fragPosWorld = positionWorld.xyz;
    fragColor = color;
    uv_out = uv;

    outShadowCoord = ( biasMat * global_ubo.lightMVP * push.modelMatrix ) * vec4(inPos, 1.0);	
}