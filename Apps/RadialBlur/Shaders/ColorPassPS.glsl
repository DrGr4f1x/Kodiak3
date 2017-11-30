#version 450
#pragma shader_stage(fragment)

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(binding = 0, set = 1) uniform texture1D gradientTex;
layout(binding = 0, set = 2) uniform sampler linearSampler;

layout(location = 0) in vec3 inColor;
layout(location = 1) in vec2 inUV;

layout(location = 0) out vec4 outFragColor;

void main()
{
	// Use max. color channel value to detect bright glow emitters
	if ((inColor.r >= 0.9f) || (inColor.g >= 0.9f) || (inColor.b >= 0.9f))
	{
		outFragColor = texture(sampler1D(gradientTex, linearSampler), inUV.x);
	}
	else
	{
		outFragColor = vec4(inColor, 1.0f);
	}
}