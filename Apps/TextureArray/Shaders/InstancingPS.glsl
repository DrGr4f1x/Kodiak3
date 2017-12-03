#version 450
#pragma shader_stage(fragment)

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(binding = 0, set = 1) uniform texture2DArray texArray;
layout(binding = 0, set = 2) uniform sampler linearSampler;

layout(location = 0) in vec3 inUV;

layout(location = 0) out vec4 outFragColor;

void main()
{
	outFragColor = texture(sampler2DArray(texArray, linearSampler), inUV);
}