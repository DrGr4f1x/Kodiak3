#version 450

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inColor;

layout(push_constant) uniform UboView
{
	mat4 projection;
	mat4 view;
	mat4 model;
} pushConsts;

layout(location = 0) out vec3 outColor;

out gl_PerVertex
{
	vec4 gl_Position;
};

void main()
{
	outColor = inColor;
	mat4 modelView = pushConsts.view * pushConsts.model;
	vec3 worldPos = vec3(modelView * vec4(inPos, 1.0));
	gl_Position = pushConsts.projection * modelView * vec4(inPos.xyz, 1.0);
}