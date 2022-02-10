#version 450

layout (location = 0) in vec3 fragColor;
layout (location = 1) in vec3 fragPosWorld;
layout (location = 2) in vec3 fragNormalWorld;

layout (location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform GlobalUbo {
  mat4 projection;
  mat4 view;
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

void main(){

    // outColor = vec4(object_ubo.color, 1.0);
    outColor = vec4(0.5, 1.0, 1.0, 1.0);
}