#version 450
#pragma shader_stage(vertex)

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec2 inUV;

struct Instance
{
	mat4 model;
	vec4 arrayIndex;
};

layout(binding = 0) uniform UBO
{
	mat4 viewProjection;
	Instance instance[8];
} ubo;

layout(location = 0) out vec3 outUV;

void main()
{
	outUV = vec3(inUV, ubo.instance[gl_InstanceIndex].arrayIndex.x);
	
	mat4 modelProjection = ubo.viewProjection * ubo.instance[gl_InstanceIndex].model;

	gl_Position = modelProjection * vec4(inPos, 1.0f);
}
