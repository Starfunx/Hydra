#version 450

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 color;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec2 uv;

layout(set = 0, binding = 0) uniform GlobalUbo {
  mat4 projection;
  mat4 view;

  vec4 ambientLightColor; // w is intensity
  vec3 lightPosition;
  vec4 lightColor; // w is intensity
} ubo;

layout (location = 0) out vec3 TexCoords;

void main() 
{
	TexCoords = inPos;

  
  mat4 viewMat = ubo.view;
  viewMat[3] = vec4(0.0, 0.0, 0.0, 1.0);
	// vec4 pos = ubo.projection * ubo.view * vec4(position.xyz, 1.0);
  vec4 pos = (ubo.projection * viewMat * vec4(inPos.xyz, 0.0));
  gl_Position = pos.xyzz;
}





