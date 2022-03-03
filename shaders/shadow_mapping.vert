#version 450

layout (location = 0) in vec3 inPos;

out gl_PerVertex 
{
    vec4 gl_Position;   
};

layout(set = 0, binding = 0) uniform GlobalUbo {
  mat4 projection;
  mat4 view;

  vec3 directionalLightDirection; 
  vec4 ambientLightColor; // w is intensity
  
  vec3 lightPosition;
  vec4 lightColor; // w is intensity
} global_ubo;

layout (push_constant) uniform Push {
    mat4 modelMatrix;
    mat4 normalMatrix;
} push;


void main()
{
	// mat4 depthMVP = push.modelMatrix * global_ubo.view * global_ubo.projection;
	mat4 depthMVP = global_ubo.projection * global_ubo.view * push.modelMatrix;
	gl_Position = depthMVP * vec4(inPos, 1.0);
}